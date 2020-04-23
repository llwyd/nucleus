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
#include "apikey.h"
#include "comms.h"
#include "sensor.h"


#define bool_t bool

#define RAW_WEATHER_SIZE ( 2048 )
#define WEATHER_URL_SIZE ( 128 )

// Settings flags
static bool_t printColours = false;
static bool_t transmitOutput = false;

uint8_t main( int argc, char ** argv )
{
	int inputFlags;
	float temp;
	float outsideTemp;
	float humidity = 0.0f; /* current unused */
	uint8_t rawWeatherData[ RAW_WEATHER_SIZE ];
	uint8_t weatherUrl[ WEATHER_URL_SIZE ];

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

	snprintf(weatherUrl,WEATHER_URL_SIZE,"/data/2.5/weather?q=%s&appid=%s",weatherLocation,weatherKey);
	
	Comms_Get("api.openweathermap.org","80",weatherUrl,rawWeatherData,RAW_WEATHER_SIZE);
	Comms_ExtractTemp( rawWeatherData, &outsideTemp);
	Sensor_Read( &temp );
	if( transmitOutput )
	{	
		uint8_t httpData[512];
		memset( httpData, 0U, 512 );
		Comms_FormatTempData( deviceUUID, &temp, &humidity, httpData, 512);
		Comms_Post(ip,port,"/raw",httpData);
		memset( httpData, 0U, 512 );
		Comms_FormatTempData( weatherUUID, &outsideTemp, &humidity, httpData, 512);
		Comms_Post(ip,port,"/raw",httpData);
	}
	
	printf( "          Device ID:	%s\r\n", deviceUUID);
	printf( "           Location:	%s\r\n", weatherLocation);
	printf(	"       Outside Temp:	%.2foC\r\n", outsideTemp);
	printf( "        Inside Temp: 	%.2foC\r\n",temp );
	return 0;
}
