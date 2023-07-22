#include "environment.h"
#include "bme280.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"

#define I2C_BAUDRATE ( 100000U )
#define I2C_TIMEOUT  ( 500000U )

static struct   bme280_dev dev;
static uint8_t  bme280_addr = BME280_I2C_ADDR_PRIM;
static struct   bme280_data bme280_rawData;

static void BME280_Setup( void );
static void BME280_Configure( void );

#define SDA_PIN (16U)
#define SCL_PIN (17U)

static void ConfigureI2C(void)
{
    printf("Initializing I2C\n");
    i2c_init(i2c_default, I2C_BAUDRATE );
    
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    bi_decl(bi_2pins_with_func(SDA_PIN, SCL_PIN, GPIO_FUNC_I2C));
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

void BME280_Delay(uint32_t period, void *intf_ptr)
{
    sleep_us((uint64_t)period);
}

int8_t BME280_I2CRead(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;
    uint8_t addr_w = ( address << 1) & 0xFE;
    uint8_t addr_r = ( address << 1) & 0x1;
    
    int8_t rslt = 0U;

    int ret0 = i2c_write_timeout_us( i2c_default,
                                    address,
                                    &reg_addr,
                                    1U,
                                    true,
                                    I2C_TIMEOUT );
    if( ret0 < 0 )
    {
        printf("I2C Read fail\n");
        rslt = -1;
    }

    int ret1 = i2c_read_timeout_us(      i2c_default,
                                    address,
                                    reg_data,
                                    len,
                                    false,
                                    I2C_TIMEOUT );

    if( ret1 < 0 )
    {
        printf("I2C Read fail\n");
        rslt = -1;
    }

    return rslt;
}

int8_t BME280_I2CWrite(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;
    uint8_t addr_w = ( address << 1) & 0xFE;
    uint8_t addr_r = ( address << 1) & 0x1;
    int8_t rslt = 0U;

    int ret = i2c_write_timeout_us( i2c_default,
                                    address,
                                    &reg_addr,
                                    1U,
                                    true,
                                    I2C_TIMEOUT );
    
    ret = i2c_write_timeout_us( i2c_default,
                                    address,
                                    reg_data,
                                    len,
                                    false,
                                    I2C_TIMEOUT );

    if( ret < 0 )
    {
        printf("I2C Write fail\n");
        rslt = -1;
    }

    return rslt;
}

static void BME280_Setup( void )
{
    int8_t rslt = BME280_OK;

    dev.intf_ptr    = &bme280_addr;
    dev.intf        = BME280_I2C_INTF;
    dev.read        = BME280_I2CRead;
    dev.write       = BME280_I2CWrite;
    dev.delay_us    = BME280_Delay;
    
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
    bme280_get_sensor_data(BME280_ALL, &bme280_rawData, &dev);
}

extern void Enviro_Print(void)
{
    printf("\tTemperature: %.2f\n", bme280_rawData.temperature);
    printf("\tHumidity: %.2f\n", bme280_rawData.humidity);
    printf("\tPressure: %.2f\n", bme280_rawData.pressure);
}

