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

static uint8_t httpRequest[ REQUEST_SIZE ];

extern void Comms_Post( uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data)
{
    int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    snprintf(httpRequest, REQUEST_SIZE,"POST %s HTTP/1.1\r\nHost: %s:%s\r\nContent-Type: application/json\r\nAccept: */*\r\nContent-Length: %d\r\nConnection:close\r\nUser-Agent: pi\r\n\r\n%s\r\n\r\n",path,ip, port, (int)strlen(data), data);


    getaddrinfo( ip, port, &hints, &servinfo );

    int sock = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    int c = connect( sock, servinfo->ai_addr, servinfo->ai_addrlen);

    int snd = send( sock, httpRequest, strlen(httpRequest), 0);

    freeaddrinfo(servinfo);

}

extern void Comms_Get(  uint8_t * ip, uint8_t * port, uint8_t * path, uint8_t * data, uint16_t len)
{
    int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;

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
}
