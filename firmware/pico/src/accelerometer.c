#include "accelerometer.h"
#include "node_events.h"
#include "emitter.h"
#include "i2c.h"
#include <assert.h>
#include "hardware/gpio.h"

#define INT1_PIN (14U)
#define INT2_PIN (15U)

static const uint8_t address = 0x1D;

static void IRQ_Callback(uint gpio, uint32_t events)
{
    switch(gpio)
    {
        case INT2_PIN:
            Emitter_EmitEvent(EVENT(AccelDataReady));
            break;
        case INT1_PIN:
            Emitter_EmitEvent(EVENT(AccelMotion));
            break;
        default:
            printf("GPIO: %d\n", gpio);
            assert(false);
            break;
    }
}

static void ConfigureGPIO(void)
{
    printf("\tConfiguring GPIO\n");
    gpio_init(INT1_PIN);
    gpio_init(INT2_PIN);

    gpio_set_dir(INT1_PIN, GPIO_IN);
    gpio_set_pulls(INT1_PIN,true,false);
    gpio_set_dir(INT2_PIN, GPIO_IN);
    gpio_set_pulls(INT2_PIN,true,false);    
}

static void Configure(void)
{
    uint8_t data = 0U;

    printf("\tConfigure FF_MT_CFG\n");
    data = 0xF8;
    (void)I2C_WriteReg(0x15,&data, 1U,(void*)&address);
    
    printf("\tConfigure FF_MT_THS\n");
    data = 0x07;
    (void)I2C_WriteReg(0x17,&data, 1U,(void*)&address);
    
    printf("\tConfigure FF_MT_COUNT\n");
    data = 0x04;
    (void)I2C_WriteReg(0x18,&data, 1U,(void*)&address);
    
    printf("\tConfigure XYZ_DATA_CFG\n");
    data = 0x02;
    (void)I2C_WriteReg(0x0E,&data, 1U,(void*)&address);
 
    /* Enable Motion interrupt */
    printf("\tConfigure CTRL_REG4\n");
    data = 0x04;
    (void)I2C_WriteReg(0x2D,&data, 1U,(void*)&address);
    
    /* Set Motion interrupt to INT1 */
    printf("\tConfigure CTRL_REG5\n");
    data = 0x04;
    (void)I2C_WriteReg(0x2E,&data, 1U,(void*)&address);
    
    printf("\tConfigure CTRL_REG3\n");
    data = 0x02;
    (void)I2C_WriteReg(0x2C,&data, 1U,(void*)&address);
    
    printf("\tConfigure CTRL_REG2\n");
    data = 0x02;
    (void)I2C_WriteReg(0x2B,&data, 1U,(void*)&address);
   
    printf("\tConfigure CTRL_REG1\n");
    data = 0x04;
    (void)I2C_WriteReg(0x2A,&data, 1U,(void*)&address);
    
}

extern void Accelerometer_ReadAll(void)
{
    uint8_t data = 0x0;
    (void)I2C_ReadReg(0x01, &data, 1U, (void*)&address);
    (void)I2C_ReadReg(0x02, &data, 1U, (void*)&address);
    (void)I2C_ReadReg(0x03, &data, 1U, (void*)&address);
    (void)I2C_ReadReg(0x04, &data, 1U, (void*)&address);
    (void)I2C_ReadReg(0x05, &data, 1U, (void*)&address);
    (void)I2C_ReadReg(0x06, &data, 1U, (void*)&address);
    
    (void)I2C_ReadReg(0x16, &data, 1U, (void*)&address);
    printf("Status: 0x%x\n",data);
}

extern void Accelerometer_Start(void)
{
    printf("\tAccelerometer Set Active\n");
    gpio_set_irq_enabled_with_callback(INT1_PIN, GPIO_IRQ_EDGE_FALL, true, IRQ_Callback);
    gpio_set_irq_enabled(INT2_PIN, GPIO_IRQ_EDGE_RISE, true);
    
    uint8_t data = 0x05;
    (void)I2C_WriteReg(0x2A,&data, 1U,(void*)&address);
    Accelerometer_Ack();
}

extern void Accelerometer_Ack(void)
{
    uint8_t data;
    (void)I2C_ReadReg(0x16, &data, 1U, (void*)&address);
}

extern void Accelerometer_Init(void)
{
    printf("Initialising Accelerometer\n");

    uint8_t data = 0x0;
    
    /* Read WHOAMI Register */
    (void)I2C_ReadReg(0x0D, &data, 1U, (void*)&address);
    
    if( data == 0x1A )
    {
        printf("\tAccelerometer WHOAMI detected\n");
        ConfigureGPIO();
        Configure();
    }
    else
    {
        assert(false);
    }
    (void)I2C_ReadReg(0x0C, &data, 1U, (void*)&address);
    printf("\tINT status: 0x%x\n",data);
}
