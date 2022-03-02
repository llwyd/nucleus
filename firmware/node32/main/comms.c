#include <stdio.h>
#include <stdlib.h>
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

#include "assert.h"

#include "comms.h" 
#include "dataqueue.h"
#include "mqtt.h"
#include "secretkeys.h"
#include "types.h"


static EventGroupHandle_t s_wifi_event_group;
static const char * WIFI_TAG = "station";

static bool wifiConnected = false;

static void ExtractAndTransmit( QueueHandle_t * queue, char * buffer );
static void ExtractFloatData( const char * inputBuffer, float * outputValue, const char * keyword );
static void ExtractStringData( const char * inputBuffer, char * outputBuffer, const char * keyword );

extern bool Comms_WifiConnected( void )
{
    return wifiConnected;
}

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
    }
}

static void Init( void )
{
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
    MQTT_Init();
}

extern void Comms_Task( void * pvParameters )
{
    char mqttTopic[64] = {0x00};
    char dataString[32] = {0x00};

    dq_data_t sensorData;
    QueueHandle_t * sensorQueue = (QueueHandle_t *)pvParameters;

    Init();
    esp_mqtt_client_handle_t * mqtt = MQTT_GetClient();
    while( 1U )
    {
        if( xQueueReceive( *sensorQueue, &sensorData, (TickType_t)10 ) == pdPASS )
        {
            MQTT_ConstructPacket( &sensorData, mqttTopic, 64U, dataString, 32U );
            
            if( MQTT_BrokerConnected() )
            {
                assert( mqtt != NULL );
                printf("transmitting: %s\n", mqttTopic);
                esp_mqtt_client_publish( *mqtt, mqttTopic, dataString, 0, 0, 0);
            }
        }
    } 
}

extern esp_err_t Comms_HTTPEventHandler( esp_http_client_event_t *evt )
{
    switch( evt->event_id )
    {
        case HTTP_EVENT_ERROR:
            printf("HTTP Error!\n");
            break;
        case HTTP_EVENT_ON_DATA:
            printf("%.*s\n", evt->data_len, (char*)evt->data);
            QueueHandle_t * dataQueue = evt->user_data;
            ExtractAndTransmit( dataQueue, evt->data );
            break;
        default:
            printf("Unhandled HTTP Event\n");
            break;
    }

    return ESP_OK;
}

static void ExtractStringData( const char * inputBuffer, char * outputBuffer, const char * keyword )
{
    char quotedKeyword[32] = {0};

    strcat(quotedKeyword, "\"");
    strcat(quotedKeyword, keyword);
    strcat(quotedKeyword, "\":");

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

static void ExtractFloatData( const char * inputBuffer, float * outputValue, const char * keyword )
{
    char quotedKeyword[32] = {0};

    strcat(quotedKeyword, "\"");
    strcat(quotedKeyword, keyword);
    strcat(quotedKeyword, "\"");

    char * p = strstr( inputBuffer, quotedKeyword );
    if( p!= NULL )
	{
        p+=(int)strlen(quotedKeyword);
        char * pch = strchr(p,',');
        *pch = '\0';
        *outputValue = (float)strtod( p + 1, NULL );
        *pch = ',';
	}

}

static void ExtractAndTransmit( QueueHandle_t * queue, char * buffer )
{
    char dataString[32] = {0};
    float val = 0.0f;

    ExtractStringData( buffer, dataString, "description" );
    printf("Outside Description: %s\n", dataString );
    DQ_AddDataToQueue( queue, dataString, dq_data_str, dq_desc_weather, "out_desc");  

    ExtractFloatData( buffer, &val, "temp" );
    printf("Outside Temperature: %.2f\n", val );
    DQ_AddDataToQueue( queue, &val, dq_data_float, dq_desc_temperature, "out_temp");  

    ExtractFloatData( buffer, &val, "humidity" );
    printf("Outside Humidity: %.2f\n", val );
    DQ_AddDataToQueue( queue, &val, dq_data_float, dq_desc_humidity, "out_hum");  
}

