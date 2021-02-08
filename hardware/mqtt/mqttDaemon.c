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
    mqtt_state_Idle,
    mqtt_state_Publish,
    mqtt_state_Disconnect,
    mqtt_state_Ping, 
} mqtt_state_t;

typedef struct mqtt_func_t
{
    mqtt_state_t state;
    mqtt_state_t (*mqtt_fn)(void);
} mqtt_func_t;

static int sock;
static struct pollfd mqtt_poll;

#define BUFFER_SIZE (128)

uint8_t sendBuffer[BUFFER_SIZE];
uint8_t recvBuffer[BUFFER_SIZE];

static const uint8_t * client_name      = "pi-livingroom";
static const uint8_t * parent_topic     = "livingroom";


mqtt_state_t MQTT_Connect( void );
mqtt_state_t MQTT_Subscribe( void );
mqtt_state_t MQTT_Idle( void );
mqtt_state_t MQTT_Publish( void );
mqtt_state_t MQTT_Disconnect( void );
mqtt_state_t MQTT_Ping( void );


typedef enum mqtt_msg_type_t
{
    mqtt_msg_Connect,
    mqtt_msg_Subscribe,
    mqtt_msg_Publish,
    mqtt_msg_Ping,
    mqtt_msg_Disconnect,
} mqtt_msg_type_t;

uint16_t MQTT_Format( mqtt_msg_type_t msg_type, uint8_t * buffer, void * msg_data );
void MQTT_Transmit( mqtt_msg_type_t msg_type, uint8_t * out, uint8_t * in, void * msg_data );

static mqtt_func_t StateTable [ 3 ] =
{
    { mqtt_state_Connect ,      MQTT_Connect },
    { mqtt_state_Subscribe ,    MQTT_Subscribe },
    { mqtt_state_Idle,          MQTT_Idle },
    //{ mqtt_state_Publish ,      MQTT_Publish },
    //{ mqtt_state_Disconnect ,   MQTT_Disconnect },
    //{ mqtt_state_Ping ,         MQTT_Ping },
};

mqtt_state_t MQTT_Connect( void )
{
    /* Default return state incase of error */
    mqtt_state_t ret = mqtt_state_Connect;

    /* ip information from localdata.h */
    const uint8_t * ip = broker_ip;
    const uint8_t * port = broker_port;    

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
                
                uint16_t full_packet_size = MQTT_Format( mqtt_msg_Connect, sendBuffer, NULL);

                int snd = send(sock, sendBuffer, full_packet_size, 0);
                if( snd < 0 )
                {
                    printf("Error Sending Data\n");
                    ret = mqtt_state_Connect;
                }
                else
                {
                    /* 5. Wait for response */
                    printf("%d Bytes Sent\n", snd);    
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
                        printf("%d Bytes Received\n", rcv);    
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

mqtt_state_t MQTT_Subscribe( void )
{
    /* Default return state incase of error */
    mqtt_state_t ret = mqtt_state_Connect;
    
    const unsigned char mqtt_template_subscribe[] =
    {
        0x82,                                                                   // message type, qos, no retain
        0x00,                                                                   // length of message
        0x00,0x01,                                                              // message identifier
    };
    
    uint8_t full_topic[64];
    memset( full_topic, 0x00, 64);

    strcat(full_topic, parent_topic);
    strcat(full_topic,"/#");

    memset(sendBuffer, 0x00, 128);
    memset(recvBuffer, 0x00, 128);

    /* Construct Packet */
    uint8_t * msg_ptr = sendBuffer;
    uint16_t packet_size = (uint16_t)sizeof( mqtt_template_subscribe );
    uint16_t topic_size = strlen(full_topic );

    memcpy( msg_ptr, mqtt_template_subscribe, packet_size );
    msg_ptr+= packet_size;
    msg_ptr++;
    *msg_ptr++ = (uint8_t)(topic_size & 0xFF);
    memcpy(msg_ptr, full_topic, topic_size);
     
    uint16_t total_packet_size = packet_size + topic_size + 1;
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
        printf("%d Bytes Sent\n", snd);    
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
            printf("%d Bytes Received\n", rcv);    
            
            if( rcv == 5 )
            {
                if(recvBuffer[0] == 0x90)
                {
                    printf("Subscription to %s Acknowledged Successfully\n", full_topic);

                    /* Create poll instance to monitor for incoming messages from broker */                    
                    mqtt_poll.fd = sock;
                    mqtt_poll.events = POLLIN;
                    memset(recvBuffer, 0x00, 128);

                    ret = mqtt_state_Idle;
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
    return ret; 
}

mqtt_state_t MQTT_Idle( void )
{
    mqtt_state_t ret = mqtt_state_Idle;
    int rv = poll( &mqtt_poll, 1, 2000);
    
    if( rv & POLLIN )
    {
        int rcv = recv(sock, recvBuffer, 256, 0U);
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
            printf("%d Bytes Received\n", rcv);    
            
            if( recvBuffer[0] == 0x30 )
            {
                uint8_t topic[64];
                uint8_t data[32];
                memset(topic,0x00,64);
                memset(data,0x00,32);
                
                unsigned char msgLen = recvBuffer[1];
                unsigned char topLen = recvBuffer[2] << 8 | recvBuffer[3];
                unsigned char * msg = &recvBuffer[4];
                
                memcpy(topic, msg, topLen);
                msg+=topLen;
                msgLen -= topLen;
                memcpy(data, msg, msgLen);
                printf("Topic:%s, Data: %s\n", topic, data);
        
                memset(recvBuffer, 0x00, 128);
            }
            ret = mqtt_state_Idle;
        }
    }
    
    return ret;
}

uint16_t MQTT_Format( mqtt_msg_type_t msg_type, uint8_t * buffer, void * msg_data )
{
    uint16_t full_packet_size = 0;

    switch( msg_type )
    {
        case mqtt_msg_Connect:
            {
            /* Message Template for mqtt connect */
            unsigned char mqtt_template_connect[] =
            {
                0x10,                                   /* connect */
                0x00,                                   /* payload size */
                0x00, 0x06,                             /* Protocol name size */
                0x4d, 0x51, 0x49, 0x73, 0x64, 0x70,     /* MQIsdp */
                0x03,                                   /* Version MQTT v3.1 */
                0x02,                                   /* Fire and forget */
                0x00, 0xb4,                             /* Keep alive timeout */
            };
            
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
            full_packet_size = total_packet_size + 2;
            }
            break;
        case mqtt_msg_Subscribe:
            break;
        case mqtt_msg_Publish:
            break;
        case mqtt_msg_Ping:
            break;
        case mqtt_msg_Disconnect:
            break;
    }
    return full_packet_size;
}

void MQTT_Transmit( mqtt_msg_type_t msg_type, uint8_t * out, uint8_t * in, void * msg_data )
{
    memset(sendBuffer, 0x00, 128);
    memset(recvBuffer, 0x00, 128);
    
    uint16_t packet_size = MQTT_Format( msg_type, out, NULL );

    switch( msg_type )
    {
        case mqtt_msg_Connect:
            break;
        case mqtt_msg_Subscribe:
            break;
        case mqtt_msg_Publish:
            break;
        case mqtt_msg_Ping:
            break;
        case mqtt_msg_Disconnect:
            break;
    }
}

void main( void )
{
    mqtt_func_t * task = StateTable;
    mqtt_state_t current_state = mqtt_state_Connect;    

    while(1)
    {
        current_state = task[current_state].mqtt_fn();
    }
}

