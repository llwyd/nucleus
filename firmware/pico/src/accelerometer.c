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
    
    gpio_set_irq_enabled_with_callback(INT1_PIN, GPIO_IRQ_EDGE_FALL, false, IRQ_Callback);
    gpio_set_irq_enabled(INT2_PIN, GPIO_IRQ_EDGE_FALL, false);
}

static void Configure(void)
{
    uint8_t data = 0U;

    data = 0xF8;
    printf("\tConfigure FF_MT_CFG (0x%x)\n", data);
    (void)I2C_WriteReg(0x15,&data, 1U,(void*)&address);
    
    data = 0x11;
    printf("\tConfigure FF_MT_THS (0x%x)\n", data);
    (void)I2C_WriteReg(0x17,&data, 1U,(void*)&address);
    
    data = 0x00;
    printf("\tConfigure FF_MT_COUNT (0x%x)\n", data);
    (void)I2C_WriteReg(0x18,&data, 1U,(void*)&address);
    
    data = 0x02;
    printf("\tConfigure XYZ_DATA_CFG (0x%x)\n", data);
    (void)I2C_WriteReg(0x0E,&data, 1U,(void*)&address);
 
    data = 0x04;
    printf("\tConfigure CTRL_REG4 (0x%x)\n", data);
    (void)I2C_WriteReg(0x2D,&data, 1U,(void*)&address);
    
    data = 0x04;
    printf("\tConfigure CTRL_REG5 (0x%x)\n", data);
    (void)I2C_WriteReg(0x2E,&data, 1U,(void*)&address);
    
    data = 0x02;
    printf("\tConfigure CTRL_REG2 (0x%x)\n", data);
    (void)I2C_WriteReg(0x2B,&data, 1U,(void*)&address);
  
}

extern void Accelerometer_Start(void)
{
    printf("\tAccelerometer: Set Active\n");    
    gpio_set_irq_enabled(INT1_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(INT2_PIN, GPIO_IRQ_EDGE_FALL, true);
    
    uint8_t data = 0x05;
    (void)I2C_WriteReg(0x2A,&data, 1U,(void*)&address);
}

extern void Accelerometer_Ack(void)
{
    uint8_t data;
    (void)I2C_ReadReg(0x16, &data, 1U, (void*)&address);
    printf("\tAccelerometer: Ack\n");
}

static void Reset(void)
{
    printf("\tAccelerometer: Resetting...");
    uint8_t data = 0x40;
    (void)I2C_WriteReg(0x2B,&data, 1U,(void*)&address);
    data = 0x0;
    do
    {
        (void)I2C_ReadRegQuiet(0x2B,&data, 1U,(void*)&address);
    }
    while(data&0x40);
    /* Wait for boot process to finish */
    printf("Complete\n");
    
    (void)I2C_ReadReg(0x0D, &data, 1U, (void*)&address);
    assert(data==0x1a);
}

extern void Accelerometer_Init(void)
{
    printf("Initialising Accelerometer\n");

    uint8_t data = 0x0;
    
    /* Read WHOAMI Register */
    (void)I2C_ReadReg(0x0D, &data, 1U, (void*)&address);
    
    if( data == 0x1A )
    {
        printf("\tAccelerometer: WHOAMI detected\n");
        Reset();
        Configure();
        ConfigureGPIO();
    }
    else
    {
        assert(false);
    }
}
