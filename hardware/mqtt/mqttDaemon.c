/*
*
*       mqtt daemon
*
*       Developing an mqtt send/recv service for main home automation system
*
*       T.L. 2021
*/

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "localdata.h"

#define MQTT_PORT ( 1883 )

typedef enum mqtt_type_t
{
    mqtt_type_float,
    mqtt_type_int16,
    mqtt_type_str,
} mqtt_type_t;

typedef union
{
    float f;
    uint16_t i;
    char * s;
} mqtt_data_t;

typedef struct mqtt_send_msg_t
{
    char * parent_topic;
    char * topic;

    mqtt_type_t type;
    mqtt_data_t data;

    double time;                /* How Often to send */
    time_t currentTime;
} mqtt_send_msg_t;

typedef struct mqtt_recv_msg_t
{
    char * parent_topic;
    char * topic;

    mqtt_type_t type;
    mqtt_data_t data;

    void (*mqtt_fn) (void); /* Action to run when receiving a given message*/
} mqtt_recv_msg_t;

typedef enum mqtt_state_t
{
    mqtt_state_Connect,
    mqtt_state_Subscribe,
    mqtt_state_Publish,
    mqtt_state_Disconnect,
    mqtt_state_Ping, 
} mqtt_state_t;


static int sock;
uint8_t sendBuffer[128];
uint8_t recvBuffer[128];

static const uint8_t * client_name = "pi-livingroom";

mqtt_state_t MQTT_Connect( void )
{
    /* Default return state incase of error */
    mqtt_state_t ret = mqtt_state_Connect;

    /* ip information from localdata.h */
    const uint8_t * ip = broker_ip;
    const uint8_t * port = broker_port;
    
    /* Message Template for mqtt connect */
    const unsigned char mqtt_template_connect[] =
    {
        0x10,                                   /* connect */
        0x00,                                   /* payload size */
        0x00, 0x06,                             /* Protocol name size */
        0x4d, 0x51, 0x49, 0x73, 0x64, 0x70,     /* MQIsdp */
        0x03,                                   /* Version MQTT v3.1 */
        0x02,                                   /* Fire and forget */
        0x00, 0x3c,                             /* Keep alive timeout */
    };

    int status;
    struct addrinfo hints, l_hints;
    struct addrinfo *servinfo, *l_servinfo;

    memset( &hints, 0U, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    /* 1. Resolve port information */
    status = getaddrinfo( ip, port, &hints, &servinfo );
    if( status != 0U )
    {
        fprintf( stderr, "Comms Error: %s\n", gai_strerror( status ) );	
        ret = mqtt_state_Connect;
    }
    else
    {   
        /* 2. Create Socket */
        sock = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if( sock < 0 )
        {
            printf("Error creating socket\n");
            ret = mqtt_state_Connect;
        }
        else
        {
            /* 3. Attempt Connection */
            int c = connect( sock, servinfo->ai_addr, servinfo->ai_addrlen);
            if( c < 0 )
            {
                printf("Connection Failed\n");
                ret = mqtt_state_Connect;
            }
            else
            {
                /* 4. Send Connect MQTT Packet */
                memset( sendBuffer, 0x00, 128 );
                memset( recvBuffer, 0x00, 128 );
                
                /* Construct Packet */
                uint8_t * msg_ptr = sendBuffer;
                uint16_t packet_size = (uint16_t)sizeof( mqtt_template_connect );
                uint16_t name_size = strlen( client_name );

                memcpy( msg_ptr, mqtt_template_connect, packet_size );
                msg_ptr+= packet_size;
                msg_ptr++;
                *msg_ptr++ = (uint8_t)(name_size & 0xFF);
                memcpy(msg_ptr, client_name, name_size);
                
                uint16_t total_packet_size = packet_size + name_size;
                sendBuffer[1] = (uint8_t)(total_packet_size&0xFF);
                uint16_t full_packet_size = total_packet_size + 2;
 
                int snd = send(sock, sendBuffer, full_packet_size, 0);
                if( snd < 0 )
                {
                    printf("Error Sending Data\n");
                    ret = mqtt_state_Connect;
                }
                else
                {
                    /* 5. Wait for response */
                    printf("%d Bytes Transmitted\n", snd);    
                    int rcv = recv(sock, recvBuffer, 128, 0);
                    
                    if( rcv < 0 )
                    {
                        printf("Error Sending Data\n");
                        ret = mqtt_state_Connect;
                    }
                    else if( rcv == 0 )
                    {
                        printf("Connection Closed\n");
                        ret = mqtt_state_Connect;
                    }
                    else
                    {
                        if( rcv == 4 )
                        {
                            if(recvBuffer[3] == 0x00)
                            {
                                printf("Connected to broker successfully\n");
                                ret = mqtt_state_Subscribe;
                            }
                            else
                            {
                                printf("Error Receiving Data\n");
                                ret = mqtt_state_Connect;
                            }
                        }
                        else
                        {
                            printf("Error Receiving Data\n");
                            ret = mqtt_state_Connect;
                        }
                    }
                }
            }
        }
    }
    
    return ret;
}


void main( void )
{
    printf("%d\n", status);
    mqtt_state_t status = MQTT_Connect();
    printf("%d\n", status);
}
