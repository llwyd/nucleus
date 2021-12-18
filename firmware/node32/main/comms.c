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
#include "mqtt_client.h"
#include "secretkeys.h"
#include "types.h"

#define MQTT_PIN ( 13U )

static EventGroupHandle_t s_wifi_event_group;
static const char * WIFI_TAG = "station";

static void MQTT_Init( void );
 
static esp_mqtt_client_handle_t mqtt_client;

static bool brokerConnected = false;
static bool wifiConnected = false;

static void ExtractAndTransmit( QueueHandle_t * queue, char * buffer );
static void MQTT_ReceiveTest ( dq_val_t * data );
static void MQTT_HandleReceivedData( const char * const topic, const uint32_t topic_len, const char * const data, const uint32_t data_len );

static uint16_t numCallbacks = 1U;
static dq_callback_t mqttCallbacks [ 1 ] = 
{
    { "debug_led", dq_data_bool, MQTT_ReceiveTest },
};

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
        MQTT_Init();
    }
}

static void Init( void )
{
    /* DEBUG led for testing mqtt receive callbacks */
    gpio_reset_pin( MQTT_PIN );
    gpio_set_direction( MQTT_PIN, GPIO_MODE_OUTPUT );
    gpio_set_level( MQTT_PIN, false );

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

static void MQTT_ReceiveTest( dq_val_t * data )
{
    printf("MQTT: Received data\n");
    gpio_set_level( MQTT_PIN, data->b );
}

static dq_val_t MQTT_Extract( char * data, dq_type_t type )
{
    dq_val_t d;

    switch( type )
    {
        case dq_data_float:
            d.f = (float)atof(data);
            break;
        case dq_data_uint32:
            d.ui = (uint32_t)atoi(data);
            break;
        case dq_data_uint16:
            d.us = (uint16_t)atoi(data);
            break;
        case dq_data_str:
            d.s = data;
            break;
        case dq_data_bool:
            if( strcmp( "1", data ) == 0U )
            {
                d.b = true;
            }
            else
            {
                d.b = false;
            }
            break;
        default:
        {
            assert( false );
            break;
        }
    }
    return d;
}

static void MQTT_HandleReceivedData( const char * const topic, const uint32_t topic_len, const char * const data, const uint32_t data_len )
{
    assert( topic != NULL );
    assert( data != NULL );

    char local_topic[64];
    char recvd_topic[64];
    char recvd_data [32];
    memset( recvd_topic, 0x00, 64);
    memset( recvd_data, 0x00, 32);

    snprintf( recvd_topic, 64, "%.*s", topic_len, topic );
    snprintf( recvd_data, 64, "%.*s", data_len, data );
    
    for( uint16_t idx = 0U; idx < numCallbacks; idx++ )
    {
        memset( local_topic, 0x00, 64);
        strcat( local_topic, MQTT_MASTER_TOPIC );
        strcat( local_topic, mqttCallbacks[idx].mqtt_label );
        
        if( strcmp( local_topic, recvd_topic ) == 0U )
        {
            dq_val_t data = MQTT_Extract( recvd_data, mqttCallbacks[idx].type );
            mqttCallbacks[idx].dq_fn( &data );
        }
    }
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
            MQTT_HandleReceivedData( event->topic, event->topic_len, event->data, event->data_len);
            break;
        default:
            printf("Unhandled event\n");
            break;
    }

}

static void ConstructMQTTPacket( dq_data_t * data, char * topic, uint16_t topicSize, char * dataBuffer, uint16_t dataBufferSize )
{
    assert( data != NULL );
    assert( topic != NULL );
    assert( dataBuffer != NULL );
    
    memset( topic, 0x00, topicSize );
    memset( dataBuffer, 0x00, dataBufferSize );
    
    strcat( topic, MQTT_MASTER_TOPIC );
    
    switch( data->type )
    { 
        case dq_data_float:
        {
            snprintf( dataBuffer, 16U, "%.3f", data->data.f);
            strcat( topic, data->mqtt_label );
            break;
        }
        case dq_data_uint32:
        {
            snprintf( dataBuffer, 16U, "%d", data->data.ui);
            strcat( topic, data->mqtt_label );
            break;
        }
        case dq_data_uint16:
        {
            snprintf( dataBuffer, 16U, "%d", data->data.us);
            strcat( topic, data->mqtt_label );
            break;
        }
        case dq_data_str:
        {
            strcat( dataBuffer, data->data.s);
            strcat( topic, data->mqtt_label );
            break;
        }
        default:
        {
            assert( false );
        }
    }    
}

extern void Comms_Task( void * pvParameters )
{
    char mqttTopic[64] = {0x00};
    char dataString[32] = {0x00};

    dq_data_t sensorData;
    QueueHandle_t * sensorQueue = (QueueHandle_t *)pvParameters;

    Init();

    while( 1U )
    {
        if( xQueueReceive( *sensorQueue, &sensorData, (TickType_t)0 ) == pdPASS )
        {
            ConstructMQTTPacket( &sensorData, mqttTopic, 64U, dataString, 32U );
            
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

