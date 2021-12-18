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

static esp_mqtt_client_handle_t mqtt_client;
static bool brokerConnected = false;
static uint16_t numCallbacks = 1U;

static void ReceiveTest( dq_val_t * data );

static dq_callback_t mqttCallbacks [ 1 ] = 
{
    { "debug_led", dq_data_bool, ReceiveTest },
};

static void ReceiveTest( dq_val_t * data )
{
    printf("MQTT: Received data\n");
    gpio_set_level( MQTT_PIN, data->b );
}

extern bool MQTT_BrokerConnected( void )
{
    return brokerConnected;
}

extern esp_mqtt_client_handle_t * MQTT_GetClient( void )
{
    return &mqtt_client;
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

extern void MQTT_ConstructPacket( dq_data_t * data, char * topic, uint16_t topicSize, char * dataBuffer, uint16_t dataBufferSize )
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

extern void MQTT_Init( void )
{
    /* DEBUG led for testing mqtt receive callbacks */
    gpio_reset_pin( MQTT_PIN );
    gpio_set_direction( MQTT_PIN, GPIO_MODE_OUTPUT );
    gpio_set_level( MQTT_PIN, false );
    
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

