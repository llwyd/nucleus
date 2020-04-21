/*
    Program for retrieving UUIDs so it can be piped to other programs
*/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>



int main( void )
{
    int status;
	struct addrinfo hints;
	struct addrinfo *servinfo;
	uint8_t tempString[ 16 ];

	uint8_t httpData[ 64 ];
	uint8_t httpRequest[ 2048 ];
    uint8_t rcvbuff[ 2048 ];

    const uint8_t * ip = "httpbin.org";
    const uint8_t * port = "80";

   	snprintf(httpRequest,sizeof(httpRequest),"GET /uuid HTTP/1.1\r\nHost: %s:%s\r\nAccept: */*\r\nUser-Agent: pi\r\n\r\n",ip, port);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo( ip, port, &hints, &servinfo );
    int s = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    int c = connect( s, servinfo->ai_addr, servinfo->ai_addrlen);

    int se = send(s,httpRequest,strlen(httpRequest),0);

    int r = recv( s, &rcvbuff, 2048, 0U);

    freeaddrinfo(servinfo);

    // Find uuid in response
    uint8_t * p = strstr( rcvbuff, "\"uuid\"" );

    if(p!=NULL)
    {
        uint8_t * pch = strtok(p,"\"");
        while( pch!=NULL )
        {
            /* magic number of uuid length */
            if(strlen(pch) == 36U )
            {
                printf ("%s\n",pch);
            }
            pch = strtok (NULL, "\"");
        }
    }
    else
    {
        fprintf( stderr, "UUID not found.\n");
    }

    return 0;
}
