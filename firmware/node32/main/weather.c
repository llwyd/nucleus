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

#define WEATHER_PIN ( 21U )

static void Init( void )
{
    gpio_reset_pin( WEATHER_PIN );
    gpio_set_direction( WEATHER_PIN, GPIO_MODE_INPUT );
    gpio_pullup_en( WEATHER_PIN );
}

extern void Weather_Task( void * pvParameters )
{
    TickType_t xLastWaitTime = xTaskGetTickCount();
    QueueHandle_t * sensorQueue = (QueueHandle_t *)pvParameters;

    Init();
    esp_http_client_config_t config =
    {
        .url            = WEATHER_URL,
        .event_handler  = Comms_HTTPEventHandler,
        .buffer_size    = 1024,
        .user_data      = sensorQueue,
    };
    
    bool weatherPin = 0;
    while( 1U )
    {
        weatherPin = (bool) gpio_get_level( WEATHER_PIN );

        /* Only attempt to get weather if wifi connected and pin is not grounded */
        if( Comms_WifiConnected() && weatherPin )
        {
            esp_http_client_handle_t client = esp_http_client_init(&config);
            esp_err_t err = esp_http_client_perform(client);
            esp_http_client_cleanup(client);
            
            vTaskDelayUntil( &xLastWaitTime, 60000 / portTICK_PERIOD_MS );
        }
        else
        {
            vTaskDelayUntil( &xLastWaitTime, 1000 / portTICK_PERIOD_MS );
        }
    }
}
