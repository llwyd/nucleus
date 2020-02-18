/*
 *	TMP102 Sensor Reading
 *	T.Lloyd 2020
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <inttypes.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>


#define bool_t bool

// Scaling factor for values read from the TMP102
static float tempScaling = 0.0625f;

// I2C device template
static uint8_t * device = "/dev/i2c-1";

// Settings flags
static bool_t printColours = false;
static bool_t transmitOutput = false;


uint8_t main( int argc, char ** argv )
{
	int inputFlags;
	while((inputFlags = getopt( argc, argv, "ct:h:")) != -1)
	{
		switch(inputFlags)
		{
			case 'c':
				printColours = true;
				break;
			case 't':
				transmitOutput = true;
				printf("Attempting transmission to %s\n",optarg);
				break;
			case 'h':
				printf("Help");
				break;
		}
	}

	
	//TMP 102
	uint16_t file = open( device, O_RDWR );
	uint8_t data[ 2 ]={ 0x00 };
	
	if( file < 0 )
	{
		printf("Error opening device");
		return -1;
	}
	uint8_t address = 0x48;
	if( ioctl( file, I2C_SLAVE, address ) < 0 )
	{
		printf("Failed to access device\r\n");
		return -1;
	}
	if( read( file, data, 2U ) != 2U )
	{
		printf( "Failed to read data\r\n" );
		return -1;
	}
	
	uint16_t rawTemp = ( (( uint16_t )data[ 0 ] << 4) | data[ 1 ] >> 4 );
	float temp = ( ( float )rawTemp ) * tempScaling;
	printf( "Temperature: %.2foC\r\n",temp );	
	close( file );
	return 0;
}
