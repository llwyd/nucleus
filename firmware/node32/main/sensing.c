#include "sensing.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

#include "sdkconfig.h"

#include "stdio.h"


/* current temperature value */
static float temperature = 0.0f;


/* i2c config */
static int i2c_master_port = 0U;

static int taskDelay = 1000;

static float TMP102_Read( void );

static QueueHandle_t * xDataQueue;

void Sensing_Init( QueueHandle_t * xTemperature )
{
    i2c_config_t conf =
    {
        .mode               = I2C_MODE_MASTER,
        .sda_io_num         = 23U,
        .sda_pullup_en      = GPIO_PULLUP_DISABLE,
        .scl_io_num         = 22U,
        .scl_pullup_en      = GPIO_PULLUP_DISABLE,
        .master.clk_speed   = 100000U,
    };

    i2c_param_config( i2c_master_port, &conf );
    i2c_driver_install( i2c_master_port, conf.mode, 0U, 0U, 0U );

    xDataQueue = xTemperature;
}

extern void Sensing_Task( void * pvParameters )
{
    TickType_t xLastWaitTime = xTaskGetTickCount();


    while( 1U )
    {
        vTaskDelayUntil( &xLastWaitTime, taskDelay / portTICK_PERIOD_MS );

        /* TODO - critical secion */
        temperature = TMP102_Read();

        xQueueSend( *xDataQueue, ( void *)&temperature, (TickType_t) 0U );

        printf("Temperature: %.3f\n", temperature);
    }
}

const float * Sensing_GetTemperature( void )
{
    return &temperature;
}

static float TMP102_Read( void )
{
    uint8_t data[2]             = {0U};
    const uint8_t address       = 0x48;
    const float scaling         = 0.0625f;

    float currentTemperature    = 0.0f;
    uint16_t rawTemperature     = 0x0U;


    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start( cmd );
    i2c_master_write_byte( cmd, ( address << 1) | 0x1, 0x1 );

    i2c_master_read(cmd, &data[0], 1, 0x0 );
    i2c_master_read(cmd, &data[1], 1, 0x1 );
    i2c_master_stop(cmd);
    
    i2c_master_cmd_begin( i2c_master_port, cmd, 100 / portTICK_PERIOD_MS );
    i2c_cmd_link_delete(cmd);

    rawTemperature = ( (( uint16_t )data[ 0 ] << 4) | data[ 1 ] >> 4 );
    currentTemperature = ( ( float )rawTemperature ) * scaling;

    return currentTemperature;
}

