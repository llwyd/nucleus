#include "environment.h"
#include "bme280.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#define I2C_BAUDRATE ( 400000U )

static struct   bme280_dev dev;
static uint8_t  bme280_addr = BME280_I2C_ADDR_PRIM;
static struct   bme280_data bme280_rawData;

static void BME280_Setup( void );
static void BME280_Configure( void );

static void ConfigureI2C(void)
{
    printf("Initializing I2C\n");
    i2c_init(i2c_default, I2C_BAUDRATE );
    
    gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
    gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

    bi_decl(bi_2pins_with_func(PICO_DEFAULT_I2C_SDA_PIN, PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C));
}

static void BME280_Configure( void )
{
    printf("Initializing BME280\n");

    uint8_t settings_sel;
    int8_t rslt = BME280_OK;

    dev.settings.osr_h     = BME280_OVERSAMPLING_1X;
    dev.settings.osr_p     = BME280_OVERSAMPLING_16X;
    dev.settings.osr_t     = BME280_OVERSAMPLING_1X;
    dev.settings.filter    = BME280_FILTER_COEFF_2;
    dev.settings.standby_time = BME280_STANDBY_TIME_250_MS;

    settings_sel = BME280_OSR_PRESS_SEL;
    settings_sel |= BME280_OSR_TEMP_SEL;
    settings_sel |= BME280_OSR_HUM_SEL;
    settings_sel |= BME280_STANDBY_SEL;
    settings_sel |= BME280_FILTER_SEL;
    
    rslt = bme280_set_sensor_settings(settings_sel, &dev);
    rslt = bme280_set_sensor_mode(BME280_NORMAL_MODE, &dev);

    if( rslt != BME280_OK )
    {
        printf("BME280 Configure FAIL\n");
    }
    else 
    {
        printf("BME280 Configure OK\n");
    }
}

static void BME280_Setup( void )
{
    int8_t rslt = BME280_OK;

    /*
    dev.intf_ptr    = &bme280_addr;
    dev.intf        = BME280_I2C_INTF;
    dev.read        = BME280_I2CRead;
    dev.write       = BME280_I2CWrite;
    dev.delay_us    = BME280_Delay;
    */
    rslt = bme280_init(&dev);
    if( rslt != BME280_OK )
    {
        printf("BME280 Init FAIL\n");
    }
    else
    {
        printf("BME280 Init OK\n");
        BME280_Configure();
    }
}

extern void Enviro_Init(void)
{
    ConfigureI2C();
    BME280_Setup();
    printf("Initialised Enviro Sensor\n");
}

extern void Enviro_Read(void)
{

}

