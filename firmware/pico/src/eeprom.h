#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>
#include <stdbool.h>

extern void EEPROM_Test(void);
extern void EEPROM_Write(uint8_t * buffer, uint16_t len);
extern void EEPROM_Read(uint8_t * buffer, uint16_t len);

#endif /* EEPROM_H_ */
