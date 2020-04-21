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

static void TransmitTemperatureData( uint8_t * ip, uint8_t * port, float * temp)
{
	int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	uint8_t tempString[ 16 ];

	uint8_t httpData[ 64 ];
	uint8_t httpRequest[ 2048 ];



	snprintf(httpData,sizeof(httpData),"{\"temperature\": \"%.2f\"}", *temp);
	printf("%s\n",httpData);

//	snprintf(httpRequest,sizeof(httpRequest),"POST HTTP/1.1\r\nHost:raw\r\nContent-Type: application/json\r\nAccept: */*\r\nContent-Length: %d\r\nConnection:close\r\nUser-Agent: pi\r\n\r\n%s\r\n\r\n",strlen(httpData),httpData);
	snprintf(httpRequest,sizeof(httpRequest),"POST /raw  HTTP/1.1\r\nHost: %s:%s\r\nContent-Type: application/json\r\nAccept: */*\r\nContent-Length: %d\r\nConnection:close\r\nUser-Agent: pi\r\n\r\n%s\r\n\r\n",ip, port, strlen(httpData), httpData);


	printf("\n%s\n",httpRequest);
	snprintf(tempString,16,"%f",*temp);
	if( port != NULL )
	{
		printf("Attempting Connection to %s:%s\n",ip,port);

		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;

		// Populate server information structure
		if( !getaddrinfo( ip, port, &hints, &servinfo ) )
		{
			printf("Got addrinfo\r\n");
			// Create the socket
			int s;
			if( ( s = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol)) != -1)
				{
					// Attempt connection
					int c = connect( s, servinfo->ai_addr, servinfo->ai_addrlen);

					printf("Connected\n");
					int se = send(s,httpRequest,strlen(httpRequest),0);
					printf("Send\n");
					freeaddrinfo(servinfo);
				}
		}
	}
	else
	{
		fprintf( stderr, "Error! Missing Port Number\n");
	}
}

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
	uint8_t getTest[2048];
	Comms_Get("httpbin.org","80","/uuid",getTest,2048);

	Sensor_Read( &temp );
	if( transmitOutput )
	{
		TransmitTemperatureData( ip, port, &temp);
	}
	


	printf( "Temperature: %.2foC\r\n",temp );	
	return 0;
}
