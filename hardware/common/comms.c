#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include "comms.h"

#define REQUEST_SIZE ( 2048 )
#define ABSOLUTE_ZERO ( 273.15 )

static const uint8_t * magicKey = "\"temp\":";

static uint8_t httpRequest[ REQUEST_SIZE ];

/* 	This function searchers for the magic key, then replaces the following comma
	with a \0.  I'm quite chuffed with this hack tbh */
extern void Comms_ExtractTemp( uint8_t * buffer, float * temp)
{
	uint8_t * p = strstr( buffer, magicKey );
	if( p!= NULL )
	{
		p+=(int)strlen(magicKey);
		uint8_t * pch = strchr(p,',');
		*pch = '\0';
		*temp = atof( p );
		*temp -= ABSOLUTE_ZERO;
	}
}

extern void Comms_FormatTempData( const uint8_t * deviceID, float * temp, float * humidity, uint8_t * buffer,uint16_t len)
{
    snprintf(buffer,len,"{\"device_id\":\"%s\",\"temperature\": \"%.2f\",\"humidity\":\"%.2f\"}", deviceID, *temp, *humidity);
}

extern void Comms_Post( uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data)
{
    int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;

	memset( &hints, 0U, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    snprintf(httpRequest, REQUEST_SIZE,"POST %s HTTP/1.1\r\nHost: %s:%s\r\nContent-Type: application/json\r\nAccept: */*\r\nContent-Length: %d\r\nConnection:close\r\nUser-Agent: pi\r\n\r\n%s\r\n\r\n",path,ip, port, (int)strlen(data), data);


    if( ( status = getaddrinfo( ip, port, &hints, &servinfo ) ) !=0U )
	{
		fprintf( stderr, "Comms Error: %s\n", gai_strerror( status ) );	
	}

    int sock = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    int c = connect( sock, servinfo->ai_addr, servinfo->ai_addrlen);

    int snd = send( sock, httpRequest, strlen(httpRequest), 0);

    freeaddrinfo(servinfo);

    close( sock );
}

extern void Comms_Get(  uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data, uint16_t len)
{
    int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;


	memset( &hints, 0U, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    snprintf(   httpRequest,
                REQUEST_SIZE,
                "GET %s HTTP/1.1\r\nHost: %s:%s\r\nAccept: */*\r\nUser-Agent: pi\r\n\r\n",path,ip,port);

    getaddrinfo( ip, port, &hints, &servinfo );

    int sock = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    int c = connect( sock, servinfo->ai_addr, servinfo->ai_addrlen);

    int snd = send( sock, httpRequest, strlen(httpRequest), 0);

    int rcv = recv( sock, data, len, 0U);

    freeaddrinfo(servinfo);

    close( sock );
}
