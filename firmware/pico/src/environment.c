#include "environment.h"
#include "bme280.h"

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include "hardware/i2c.h"
#include <string.h>

#define I2C_BAUDRATE ( 100000U )
#define I2C_TIMEOUT  ( 500000U )

static struct   bme280_dev dev;
static uint8_t  bme280_addr = BME280_I2C_ADDR_PRIM;
static struct   bme280_data bme280_rawData;
static struct   bme280_settings settings;

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

    int8_t rslt = BME280_OK;

    settings.osr_h     = BME280_OVERSAMPLING_1X;
    settings.osr_p     = BME280_OVERSAMPLING_16X;
    settings.osr_t     = BME280_OVERSAMPLING_1X;
    settings.filter    = BME280_FILTER_COEFF_2;
    settings.standby_time = BME280_STANDBY_TIME_0_5_MS;

    rslt = bme280_set_sensor_settings(BME280_SEL_ALL_SETTINGS, &settings, &dev);
    if( rslt != BME280_OK )
    {
        printf("BME280 Configure FAIL\n");
    }
    else 
    {
        printf("BME280 Configure OK\n");
    }
    
    
    rslt = bme280_set_sensor_mode(BME280_POWERMODE_NORMAL, &dev);

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
    sleep_us(period);
}

int8_t BME280_I2CRead(uint8_t reg_addr, uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;
    int8_t rslt = 0U;

    printf("Expecting to read %d bytes, ", len);
    int ret0 = i2c_write_blocking( i2c_default,
                                    address,
                                    &reg_addr,
                                    1U,
                                    true
                                     );
    if( ret0 < 0 )
    {
        printf("I2C Read fail\n");
        rslt = -1;
    }
   
    int ret1 = i2c_read_blocking(i2c_default,address,reg_data,len,false);
    
    if( ret1 < 0 )
    {
        printf("I2C Read fail\n");
        rslt = -1;
    }
    else
    {
        printf("%d bytes read\n", ret1);
    }
    

    return rslt;
}

int8_t BME280_I2CWrite(uint8_t reg_addr, const uint8_t *reg_data, uint32_t len, void *intf_ptr)
{
    uint8_t address = *(uint8_t *)intf_ptr;
    int8_t rslt = 0U;
    uint8_t buffer[32] = {0};
    memset(buffer, 0x00, 32);

    buffer[0] = reg_addr;
    memcpy(&buffer[1], reg_data, len );
    printf("Expecting to write %d bytes, ", len + 1U);
     
    int ret = i2c_write_blocking( i2c_default,
                                    address,
                                    buffer,
                                    len + 1U,
                                    true
                                     );
    if( ret < 0 )
    {
        printf("I2C Read fail\n");
        rslt = -1;
    }
    else
    {
        printf("%d bytes written\n", ret);
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
    int8_t rslt = bme280_get_sensor_data(BME280_ALL, &bme280_rawData, &dev);
    if( rslt != BME280_OK )
    {
        printf("BME280 Sensor Read FAIL\n");
    }
    else 
    {
        printf("BME280 Sensor Read OK\n");
    }
}

extern void Enviro_Print(void)
{
    /*
    printf("\tTemperature: %.2f\n", bme280_rawData.temperature);
    printf("\tHumidity: %.2f\n", bme280_rawData.humidity);
    printf("\tPressure: %.2f\n", bme280_rawData.pressure);
    */
    printf("\tTemperature: %d (0x%x)\n", bme280_rawData.temperature, bme280_rawData.temperature);
    printf("\tHumidity: %d (0x%x)\n", bme280_rawData.humidity, bme280_rawData.humidity);
    printf("\tPressure: %d (0x%x)\n", bme280_rawData.pressure, bme280_rawData.pressure);
}

