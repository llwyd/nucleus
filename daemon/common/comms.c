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


extern void Comms_PopulateJSON( json_data_t * strct, char * name, json_field_t * d, json_type_t d_type)
{
	memset( strct->name, 0, JSON_NAME_LEN );
	strcat( strct->name, name );
	
	switch( d_type )
	{
		case json_float:
			strct->data.f = d->f;
			strct->type = json_float;
			break;
		
		case json_int:
			strct->data.i = d->i;
			strct->type = json_int;
			break;
	}
}

extern void Comms_ExtractFloat( uint8_t * buffer,float * outputFloat, uint8_t * keyword)
{
	uint8_t * p = strstr( buffer, keyword );
	uint8_t outputBuffer[32];
	memset(outputBuffer, 0x00, 32);	

	*outputFloat = 0.0f;
	if( p!= NULL )
	{
		p+=(int)strlen(keyword);
		uint8_t * pch = strchr(p,',');
		*pch = '\0';
		strcat(outputBuffer,p+1);
		int stringLen = strlen(outputBuffer);	
		*(outputBuffer+stringLen-1) = '\0';
		*pch = ',';
	}
	*outputFloat = (float)atof(outputBuffer);
}

extern void Comms_ExtractValue( uint8_t * buffer,uint8_t * outputBuffer, uint8_t * keyword)
{
	uint8_t * p = strstr( buffer, keyword );
	if( p!= NULL )
	{
		p+=(int)strlen(keyword);
		uint8_t * pch = strchr(p,',');
		*pch = '\0';
		strcat(outputBuffer,p+1);
		int stringLen = strlen(outputBuffer);	
		*(outputBuffer+stringLen-1) = '\0';
		*pch = ',';
	}
}

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
		*pch = ',';
	}
}


extern void Comms_FormatData( uint8_t * buffer, uint16_t len, json_data_t * dataItem, int numItems )
{
	char tempBuffer[ 128 ];
	memset( tempBuffer, 0, 128 );
	memset( buffer, 0, len);

	/* insert the first json data structure */
	strcat( buffer, "{");
	for( int i =0; i < numItems; i++ )
	{
		switch( dataItem[i].type )
		{
			case json_float:
				snprintf(tempBuffer, 128, "\"%s\":\"%.1f\"",dataItem[i].name, dataItem[i].data.f);
				break;

			case json_int:
				snprintf(tempBuffer, 128, "\"%s\":\"%d\"",dataItem[i].name, dataItem[i].data.i);
				break;
		}
		strcat( buffer, tempBuffer );
		if( i < (numItems - 1))
		{
			strcat( buffer, ",");
		}
		memset( tempBuffer, 0, 128);
	}
	strcat( buffer, "}");
}

extern void Comms_FormatStringData( const uint8_t * deviceID, uint8_t * data, uint8_t * buffer,uint16_t len)
{
    snprintf(buffer,len,"{\"device_id\":\"%s\",\"weather\": \"%s\"}", deviceID, data);
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

extern bool Comms_Get(  uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data, uint16_t len)
{
    int status;
    struct addrinfo hints;
    struct addrinfo *servinfo;
    bool ret = false;
    int sock;

    memset( &hints, 0U, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    snprintf(   httpRequest,
                REQUEST_SIZE,
                "GET %s HTTP/1.1\r\nHost: %s:%s\r\nAccept: */*\r\nUser-Agent: pi\r\n\r\n",path,ip,port);

    status =getaddrinfo( ip, port, &hints, &servinfo );
    if( status != 0U )
    {
        fprintf( stderr, "Comms Error: %s\n", gai_strerror( status ) );	
        ret = false;
    }
    else
    {   
        sock = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if( sock < 0 )
        {
            printf("Error creating socket\n");
            ret = false;
        }
        else
        {
            int c = connect( sock, servinfo->ai_addr, servinfo->ai_addrlen);
            if( c < 0 )
            {
                printf("Connection Failed\n");
                ret = false;
            }
            else
            {
                int snd = send( sock, httpRequest, strlen(httpRequest), 0);
                if( snd < 0 )
                {
                    printf("Error Sending Data\n");
                    ret = false;
                }
                else
                {
                    int rcv = recv( sock, data, len, 0U);
                    if( rcv < 0 )
                    {
                        printf("FAIL\n");
                        ret = false;
                    }
                    else if( rcv == 0 )
                    {
                        printf("FAIL\n");
                        ret = false;
                    }
                    else
                    {
                        freeaddrinfo(servinfo);
                        close( sock );
                        ret = true;
                    }
                }
            }
        }
    } 
    return ret;
}
