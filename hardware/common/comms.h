#ifndef COMMS_H
#define COMMS_H

#include <inttypes.h>


#define JSON_NAME_LEN ( 64 )

typedef struct
{
	uint8_t name[ JSON_NAME_LEN ];
	union Data
	{
		float f;
		int i;
		char * c;
	} data;
}
json_data_t;


extern void Comms_FormatData( uint8_t * buffer, uint16_t len, json_data_t * dataItem, int numItems );
extern void Comms_ExtractTemp( uint8_t * buffer, float * temp);
extern void Comms_FormatTempData( const uint8_t * deviceID, float * temp, float * humidity, uint8_t * buffer,uint16_t len);
extern void Comms_Post( uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data);
extern void Comms_Get(  uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data, uint16_t len);

#endif /* COMMS_H */

