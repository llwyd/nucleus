#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "sdkconfig.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_event.h"
#include "esp_http_client.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"
#include "secretkeys.h"
#include "types.h"

static EventGroupHandle_t s_wifi_event_group;
static const char * WIFI_TAG = "station";

static void MQTT_Init( void );

static QueueHandle_t * xCommsDataQueue;
static QueueHandle_t * xSlowDataQueue;
    
static esp_mqtt_client_handle_t mqtt_client;

static bool brokerConnected = false;
static bool wifiConnected = false;


static void ExtractAndTransmit( char * buffer );

static void Wifi_EventHandler( void * arg, esp_event_base_t event_base, int32_t event_id, void* event_data )
{
    if( event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        wifiConnected = false;
        esp_wifi_connect();
    }
    else if( event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifiConnected = false;
        esp_wifi_connect();
    }
    else if( event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP )
    {
        ip_event_got_ip_t * event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(WIFI_TAG, "Got ip: " IPSTR, IP2STR( &event->ip_info.ip ) );
        wifiConnected = true;
        MQTT_Init();
    }
}

void Comms_Init( QueueHandle_t * xTemperature, QueueHandle_t * xSlowTemperature )
{
    xCommsDataQueue = xTemperature;
    xSlowDataQueue  = xSlowTemperature;
    
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }

    s_wifi_event_group = xEventGroupCreate();
    
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init( &cfg );

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_any_ip;

    esp_event_handler_instance_register(    WIFI_EVENT,
                                            ESP_EVENT_ANY_ID,
                                            &Wifi_EventHandler,
                                            NULL,
                                            &instance_any_id);
    
    esp_event_handler_instance_register(    IP_EVENT,
                                            IP_EVENT_STA_GOT_IP,
                                            &Wifi_EventHandler,
                                            NULL,
                                            &instance_any_ip);

    wifi_config_t wifi_config = {
        .sta = {
                .ssid       = WIFI_SSID,
                .password   = WIFI_PASS, 
                .pmf_cfg = {
                    .capable = true,
                    .required = false,
                },
            },
    };

    esp_wifi_set_mode( WIFI_MODE_STA );
    esp_wifi_set_config( WIFI_IF_STA, &wifi_config );
    esp_wifi_start();
}

static void MQTT_EventHandler( void * arg, esp_event_base_t event_base, int32_t event_id, void* event_data )
{
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    esp_mqtt_event_id_t currentEvent = (esp_mqtt_event_id_t)event_id;

    /* TODO, improve this */
    switch( currentEvent )
    {
        case MQTT_EVENT_CONNECTED:
            printf("Connected to broker!\n");
            brokerConnected = true;
            esp_mqtt_client_subscribe( client, MQTT_TOPIC, 0 );
            break;
        case MQTT_EVENT_DISCONNECTED:
            brokerConnected = false;
            printf("Disconnected from broker\n");
            break;
        case MQTT_EVENT_DATA:
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            break;
        default:
            printf("Unhandled event\n");
            break;
    }

}

extern void Comms_Task( void * pvParameters )
{
    float rxTemperature;
    char dataString[16] = {0x00};
    char mqttTopic[64] = {0x00};

    sensing_t sensorData;
    QueueHandle_t * sensorQueue = (QueueHandle_t *)pvParameters;

    while( 1U )
    {
        if( xQueueReceive( *sensorQueue, &sensorData, (TickType_t)100 ) == pdPASS )
        {
            memset( dataString, 0x00, 16U);
            memset( mqttTopic, 0x00, 64U );
            strcat( mqttTopic, MQTT_MASTER_TOPIC );
            switch( sensorData.type )
            {
                case sensing_type_temperature:
                    snprintf( dataString, 16U, "%.3f", sensorData.data.f);
                    strcat( mqttTopic, sensorData.mqtt_label );
                    break;
                case sensing_type_humidity:
                    snprintf( dataString, 16U, "%.3f", sensorData.data.f);
                    strcat( mqttTopic, sensorData.mqtt_label );
                    break;
                case sensing_type_pressure:
                    snprintf( dataString, 16U, "%.3f", sensorData.data.f);
                    strcat( mqttTopic, sensorData.mqtt_label );
                    break;
            }
            if( brokerConnected )
            {
                printf("transmitting: %s\n", mqttTopic);
                esp_mqtt_client_publish(mqtt_client, mqttTopic, dataString, 0, 0, 0);
            }
        } 
    } 
}

static void MQTT_Init( void )
{
    esp_mqtt_client_config_t mqtt_cfg =
    {
        .host = MQTT_BROKER,
        .port = 1883,
        .transport = MQTT_TRANSPORT_OVER_TCP,
    };

    mqtt_client = esp_mqtt_client_init( &mqtt_cfg );
    esp_mqtt_client_register_event( mqtt_client, ESP_EVENT_ANY_ID, MQTT_EventHandler, NULL );
    esp_mqtt_client_start(mqtt_client);
}



esp_err_t HTTP_EventHandler( esp_http_client_event_t *evt )
{
    switch( evt->event_id )
    {
        case HTTP_EVENT_ERROR:
            printf("HTTP Error!\n");
            break;
        case HTTP_EVENT_ON_DATA:
            printf("%.*s", evt->data_len, (char*)evt->data);
            ExtractAndTransmit( evt->data );
            break;
        default:
            printf("Unhandled HTTP Event\n");
            break;
    }

    return ESP_OK;
}

static void ExtractData( const char * inputBuffer, char * outputBuffer, const char * keyword )
{
    char quotedKeyword[32] = {0};

    strcat(quotedKeyword, "\"");
    strcat(quotedKeyword, keyword);
    strcat(quotedKeyword, "\"");

    char * p = strstr( inputBuffer, quotedKeyword );
    memset(outputBuffer, 0x00, 32);
    if( p!= NULL )
	{
        p+=(int)strlen(quotedKeyword);
        char * pch = strchr(p,',');
        *pch = '\0';
        strcat(outputBuffer,p + 1);
        int stringLen = strlen(outputBuffer);
        *(outputBuffer+stringLen-1) = '\0';
        *pch = ',';
	}

}

static void ExtractAndTransmit( char * buffer )
{
    char dataString[32] = {0};
    ExtractData( buffer, dataString, "description" );
    printf("Outside Descrption: %s\n", dataString );

    ExtractData( buffer, dataString, "temp" );
    printf("Outside Temperature: %s\n", dataString );

    ExtractData( buffer, dataString, "humidity" );
    printf("Outside Humidity: %s\n", dataString );
}

extern void Comms_Weather( void * pvParameters )
{
    TickType_t xLastWaitTime = xTaskGetTickCount();

    esp_http_client_config_t config =
    {
        .url = WEATHER_URL,
        .event_handler = HTTP_EventHandler,
    };
    char extractedData[32] = {0};
    while( 1U )
    {
        vTaskDelayUntil( &xLastWaitTime, 10000 / portTICK_PERIOD_MS );

        if( wifiConnected )
        {
            esp_http_client_handle_t client = esp_http_client_init(&config);
            esp_err_t err = esp_http_client_perform(client);
            esp_http_client_cleanup(client);
        }
    }
}

