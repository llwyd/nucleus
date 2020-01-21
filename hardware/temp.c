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
#include <unistd.h>

uint8_t main( void )
{
	//TMP 102
	float tempScaling = 0.0625f;
	uint8_t * device = "/dev/i2c-1";
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
