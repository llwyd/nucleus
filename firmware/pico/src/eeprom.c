#include "i2c.h"
#include "gpio.h"
#include <assert.h>
#include "hardware/gpio.h"
#include "eeprom.h"

static const uint32_t entry_size = 32U;
static const uint16_t eeprom_size = 256U;
static const uint8_t address = 0x50;

extern uint16_t EEPROM_GetSize(void)
{
    return eeprom_size;
}

static uint32_t GetIndex(eeprom_label_t label)
{
    assert(label < EEPROM_NONE);
    uint32_t raw_index = ((uint32_t)label) * entry_size;

    return raw_index;
}

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

extern void EEPROM_WriteRaw(uint8_t * buffer, uint16_t len, uint16_t loc)
{
    assert(buffer != NULL);
    assert((loc + len) <= eeprom_size);

    if( len <= 16 )
    {
        (void)I2C_WriteReg(loc, buffer, len, (void*)&address);
    }
    else
    {

    }
}

extern void EEPROM_ReadRaw(uint8_t * buffer, uint16_t len, uint16_t loc)
{
    assert(buffer != NULL);
    assert((loc + len) <= eeprom_size);
    memset(buffer, 0U, len);
    I2C_ReadReg(loc, buffer, len, (void*)&address);
}

extern bool EEPROM_Write(uint8_t * buffer, uint16_t len, eeprom_label_t loc)
{
    assert(buffer != NULL);
    assert(((uint16_t)loc + len) <= eeprom_size);
    assert(loc < EEPROM_NONE);
    
    uint32_t raw_index = GetIndex(loc);

    printf("Raw Index: %d\n", raw_index);
    /*
    if( len <= 16 )
    {
        (void)I2C_WriteReg(loc, buffer, len, (void*)&address);
    }
    else
    {
        uint16_t remaining_len = len;
        uint16_t bytes_written = 0U;
        while(bytes_written < len )
        {
            if( len <= 16U )
            {
                (void)I2C_WriteReg(loc, buffer, len, (void*)&address);
                bytes_written += len;
            }
            else
            {
                (void)I2C_WriteReg(loc, buffer, len, (void*)&address);
            
            }
        }
    }
    */
    return true;
}

extern bool EEPROM_Read(uint8_t * buffer, uint16_t len, eeprom_label_t loc);

