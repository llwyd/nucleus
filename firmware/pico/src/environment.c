#include "environment.h"
#include "bme280.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include "i2c.h"
#include <string.h>

#define I2C_BAUDRATE ( 400000U )
#define I2C_TIMEOUT  ( 500000U )

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
    settings.filter    = BME280_FILTER_COEFF_2;
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
    BME280_Setup();
}

extern void Enviro_Read(void)
{
    int8_t rslt = bme280_get_sensor_data(BME280_ALL, &env_data, &dev);
    if( rslt != BME280_OK )
    {
        printf("\tBME280 Sensor Read FAIL\n");
    }
    else 
    {
        printf("\tBME280 Sensor Read OK\n");
    }
}

extern void Enviro_Print(void)
{
    
    printf("\tTemperature: %.2f\n", env_data.temperature);
    printf("\tHumidity: %.2f\n", env_data.humidity);
    printf("\tPressure: %.2f\n", env_data.pressure);
    
    /*
    printf("\tTemperature: %d (0x%x)\n", env_data.temperature, env_data.temperature);
    printf("\tHumidity: %d (0x%x)\n", env_data.humidity, env_data.humidity);
    printf("\tPressure: %d (0x%x)\n", env_data.pressure, env_data.pressure);
    */
}

extern const double * const Enviro_GetTemperature(void)
{
    return &env_data.temperature;
}

extern const double * const Enviro_GetHumidity(void)
{
    return &env_data.humidity;
}

extern const double * const Enviro_GetPressure(void)
{
    return &env_data.pressure;
}

extern void Enviro_ConvertToStr(char * buffer, uint8_t buffer_len, const double * const data)
{
    assert(data != NULL );
    assert(buffer != NULL);

    memset(buffer,0x00, buffer_len);
    snprintf(buffer, buffer_len,"%.4f", *data);
}

extern void Enviro_GenerateJSON(char * buffer, uint8_t buffer_len)
{
    assert(buffer != NULL);

    memset(buffer,0x00, buffer_len);
    snprintf(buffer, buffer_len,"{\"temperature\":%.1f,\"humidity\":%.1f}",
            env_data.temperature,
            env_data.humidity);
}

