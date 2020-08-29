#ifndef COMMS_H
#define COMMS_H

#include <inttypes.h>


#define JSON_NAME_LEN ( 64 )


typedef enum json_type_t
{
	json_float,
	json_int,
	json_str,
} json_type_t;


typedef union
{
	float f;
	int i;
	char * c;
} json_field_t;


typedef struct
{
	uint8_t name[ JSON_NAME_LEN ];
	json_type_t type;
	json_field_t data;
}
json_data_t;


/* HTTP data parsing */
extern void Comms_ExtractValue( uint8_t * buffer,uint8_t * outputBuffer, uint8_t * keyword);
extern void Comms_ExtractFloat( uint8_t * buffer,float * outputFloat, uint8_t * keyword);
extern void Comms_ExtractTemp( uint8_t * buffer, float * temp);

/* JSON data formatting */
extern void Comms_PopulateJSON( json_data_t * strct, char * name, json_field_t * d, json_type_t d_type);

/* HTTP data formatting */
extern void Comms_FormatTempData( const uint8_t * deviceID, float * temp, float * humidity, uint8_t * buffer,uint16_t len);
extern void Comms_FormatData( uint8_t * buffer, uint16_t len, json_data_t * dataItem, int numItems );
extern void Comms_FormatStringData( const uint8_t * deviceID, uint8_t * data, uint8_t * buffer,uint16_t len);

/* Network Transmission */
extern void Comms_Post( uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data);
extern void Comms_Get(  uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data, uint16_t len);

#endif /* COMMS_H */

