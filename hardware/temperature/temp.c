/*
 *	TMP102 Sensor Reading
 *	T.Lloyd 2020
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <inttypes.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "comms.h"
#include "sensor.h"


#define bool_t bool

// Settings flags
static bool_t printColours = false;
static bool_t transmitOutput = false;

uint8_t main( int argc, char ** argv )
{
	int inputFlags;
	float temp;

	uint8_t * ip;
	uint8_t * port;
	
	while((inputFlags = getopt( argc, argv, "ct:p:h")) != -1)
	{
		switch(inputFlags)
		{
			case 'c':
				printColours = true;
				break;
			case 't':
				transmitOutput = true;
				ip = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case 'h':
				printf("Help");
				break;
		}
	}
	
	
	Sensor_Read( &temp );
	if( transmitOutput )
	{	
		uint8_t httpData[512];
		Comms_FormatTempData( &temp, &temp, httpData, 512);
		Comms_Post(ip,port,"/raw",httpData);
	}
	


	printf( "Temperature: %.2foC\r\n",temp );	
	return 0;
}
