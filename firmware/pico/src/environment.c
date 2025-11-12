#include "environment.h"
#include "bme280.h"
#include "MCP9808.h"

#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "i2c.h"
#include "uptime.h"
#include "alarm.h"
#include <string.h>

static struct   bme280_dev dev;
static uint8_t  bme280_addr = BME280_I2C_ADDR_PRIM;
static struct   bme280_data env_data;
static struct   bme280_settings settings;

static void BME280_Setup( void );
static void BME280_Configure( void );

static void BME280_Configure( void )
{
    int8_t rslt = BME280_OK;

    settings.osr_h     = BME280_OVERSAMPLING_1X;
    settings.osr_p     = BME280_OVERSAMPLING_16X;
    settings.osr_t     = BME280_OVERSAMPLING_1X;
    settings.filter    = BME280_FILTER_COEFF_8;
    settings.standby_time = BME280_STANDBY_TIME_0_5_MS;

    rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &dev);
    if( rslt != BME280_OK )
    {
        printf("\tBME280 Configure FAIL\n");
    }
    else 
    {
        printf("\tBME280 Configure OK\n");
    }
    
    
    rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev);

    if( rslt != BME280_OK )
    {
        printf("\tBME280 Set Mode FAIL\n");
    }
    else 
    {
        printf("\tBME280 Set Mode OK\n");
    }
}

void Delay(uint32_t period, void *intf_ptr)
{
    (void)intf_ptr;
    sleep_us(period);
}

static void BME280_Setup( void )
{
    int8_t rslt = BME280_OK;

    dev.intf_ptr    = &bme280_addr;
    dev.intf        = BME280_I2C_INTF;
    dev.read        = I2C_ReadReg;
    dev.write       = I2C_WriteReg;
    dev.delay_us    = Delay;
    
    rslt = bme280_init(&dev);
    if( rslt != BME280_OK )
    {
        printf("\tBME280 Init FAIL\n");
    }
    else
    {
        printf("\tBME280 Init OK\n");
        BME280_Configure();
    }
}

extern void Enviro_Init(void)
{
    printf("Initialising Enviro Sensor\n");
#ifdef SENSOR_MCP9808
    MCP9808_Setup();
#else
    BME280_Setup();
#endif
}

extern void Enviro_Read(void)
{
#ifdef SENSOR_MCP9808
    MCP9808_Read();
#else
    int8_t rslt = bme280_get_sensor_data(BME280_ALL, &env_data, &dev);
    if( rslt != BME280_OK )
    {
        printf("\tBME280 Sensor Read FAIL\n");
    }
    else 
    {
        printf("\tBME280 Sensor Read OK\n");
    }
#endif
    (void)Uptime_Refresh();
}

extern void Enviro_Print(void)
{ 
#ifdef SENSOR_MCP9808
    printf("\tTemperature: %.2f\n", MCP9808_GetTemperature());
    printf("\tHumidity: %.2f\n", 0.0);
    printf("\tPressure: %.2f\n", 0.0);
#else
    printf("\tTemperature: %.2f\n", env_data.temperature);
    printf("\tHumidity: %.2f\n", env_data.humidity);
    printf("\tPressure: %.2f\n", env_data.pressure);
#endif
}

extern void Enviro_GenDigest(char * buffer, uint8_t buffer_len)
{
    assert(buffer != NULL);
    uint64_t utime = (uint64_t)Alarm_GetUnixTime();

    memset(buffer,0x00, buffer_len);
    
    snprintf(buffer, buffer_len,"{\"t\":%.1f,\"h\":%.1f,\"ts\":%llu}",
            env_data.temperature,
            env_data.humidity,
            utime);
}

extern void Enviro_GenShortDigest(char * buffer, uint8_t buffer_len)
{
    assert(buffer != NULL);

    memset(buffer,0x00, buffer_len);
    
    snprintf(buffer, buffer_len,"{\"t\":%.1f,\"h\":%.1f}",
            env_data.temperature,
            env_data.humidity);
}


