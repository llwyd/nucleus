#ifndef COMMS_H
#define COMMS_H

#include <inttypes.h>


extern void Comms_FormatTempData( float * inside, float * outside, uint8_t * buffer,uint16_t len);
extern void Comms_Post( uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data);
extern void Comms_Get(  uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data, uint16_t len);

#endif /* COMMS_H */