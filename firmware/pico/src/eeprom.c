#include "i2c.h"
#include "gpio.h"
#include <assert.h>
#include "hardware/gpio.h"

static const uint8_t address = 0x50;

extern void EEPROM_Test(void)
{
    printf("\tTesting EEPROM\n");
    /* Try Read */ 
    uint8_t data = 0U;
    (void)I2C_ReadReg(0x00, &data, 1U, (void*)&address);
    printf("\tReceived: 0x%x\n", data);

    /* Try write and then read again */
    data = 0xAB;
    (void)I2C_WriteReg(0x00, &data, 1U, (void*)&address);
    
    data = 0x11;
    while( I2C_ReadReg(0x00, &data, 1U, (void*)&address) < 0);

    (void)I2C_ReadReg(0x00, &data, 1U, (void*)&address);
    printf("\tReceived: 0x%x\n", data);
}
