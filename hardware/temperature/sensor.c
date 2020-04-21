
#include <stdio.h>
#include <stdlib.h>
#include "sensor.h"
#include <linux/i2c-dev.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

// Scaling factor for values read from the TMP102
static const float tempScaling = 0.0625f;

// I2C device template
static const uint8_t * device = "/dev/i2c-1";

// TMP102 sensor address
static const uint8_t address = 0x48;

extern void Sensor_Read( float * temperature )
{
	uint16_t file = open( device, O_RDWR );
	uint8_t data[ 2 ]={ 0x00 };

	if( file < 0 )
	{
		fprintf( stderr,"Error opening device");
	}
	
	if( ioctl( file, I2C_SLAVE, address ) < 0 )
	{
		fprintf( stderr,"Failed to access device\r\n");
	}
	if( read( file, data, 2U ) != 2U )
	{
		fprintf( stderr,"Failed to read data\r\n" );
	}

	close( file );

	uint16_t rawTemp = ( (( uint16_t )data[ 0 ] << 4) | data[ 1 ] >> 4 );

	*temperature = ( ( float )rawTemp ) * tempScaling;
}
