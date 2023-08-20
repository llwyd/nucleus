#include "accelerometer.h"
#include "i2c.h"
#include <assert.h>

static const uint8_t address = 0x1D;


static void Configure(void)
{
    uint8_t data = 0U;

    printf("\tConfigure FF_MT_CFG\n");
    data = 0x78;
    (void)I2C_WriteReg(0x15,&data, 1U,(void*)&address);
    
    printf("\tConfigure FF_MT_THS\n");
    data = 0x0F;
    (void)I2C_WriteReg(0x17,&data, 1U,(void*)&address);
    
    printf("\tConfigure FF_MT_COUNT\n");
    data = 0x1F;
    (void)I2C_WriteReg(0x18,&data, 1U,(void*)&address);

    printf("\tConfigure CTRL_REG1\n");
    data = 0x01;
    (void)I2C_WriteReg(0x2A,&data, 1U,(void*)&address);
    
    /* Enable Motion interrupt */
    printf("\tConfigure CTRL_REG4\n");
    data = 0x04;
    (void)I2C_WriteReg(0x2D,&data, 1U,(void*)&address);
    
    /* Set Motion interrupt to INT1 */
    printf("\tConfigure CTRL_REG5\n");
    data = 0x04;
    (void)I2C_WriteReg(0x2D,&data, 1U,(void*)&address);
}

extern void Accelerometer_Init(void)
{
    printf("Initialising Accelerometer\n");

    uint8_t data[2U] = {0x0};
    
    /* Read WHOAMI Register */
    (void)I2C_ReadReg(0x0D, data, 1U, (void*)&address);
    
    if( data[0] == 0x1A )
    {
        printf("\tAccelerometer WHOAMI detected\n");
        Configure();
    }
    else
    {
        assert(false);
    }
}
