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


#define bool_t bool

// Scaling factor for values read from the TMP102
static const float tempScaling = 0.0625f;

// I2C device template
static const uint8_t * device = "/dev/i2c-1";

// Settings flags
static bool_t printColours = false;
static bool_t transmitOutput = false;


static void ReadTemperature( float * temperature  )
{

	uint16_t file = open( device, O_RDWR );
	uint8_t data[ 2 ]={ 0x00 };

	if( file < 0 )
	{
		printf("Error opening device");
	}
	uint8_t address = 0x48;
	if( ioctl( file, I2C_SLAVE, address ) < 0 )
	{
		printf("Failed to access device\r\n");
	}
	if( read( file, data, 2U ) != 2U )
	{
		printf( "Failed to read data\r\n" );
	}

	close( file );
	uint16_t rawTemp = ( (( uint16_t )data[ 0 ] << 4) | data[ 1 ] >> 4 );

	*temperature = ( ( float )rawTemp ) * tempScaling;
}

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

	snprintf(httpRequest,sizeof(httpRequest),"POST HTTP/1.1\r\nHost: %s\r\nContent-Type: application/json\r\nAccept: */*\r\nContent-Length: %d\r\nConnection:close\r\nUser-Agent: pi\r\n\r\n%s\r\n\r\n",ip,strlen(httpData),httpData);


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

	ReadTemperature( &temp );
	if( transmitOutput )
	{
		TransmitTemperatureData( ip, port, &temp);
	}
	


	printf( "Temperature: %.2foC\r\n",temp );	
	return 0;
}
