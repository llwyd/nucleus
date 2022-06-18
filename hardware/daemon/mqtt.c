#include "mqtt.h"

#include <time.h>
#include <assert.h>

#define MQTT_PORT ( "1883" )
#define BUFFER_SIZE (128)

#define MQTT_TIMEOUT ( 0xb4 )

static int sock;
static struct pollfd mqtt_poll;

static uint8_t send_buffer[ BUFFER_SIZE ];
static uint8_t recv_buffer[ BUFFER_SIZE ];

static uint8_t * client_name;
static uint8_t * parent_topic;

static uint8_t num_sub = 0;

static uint8_t * broker_ip;
static uint8_t * broker_port;

typedef enum mqtt_msg_type_t
{
    mqtt_msg_Connect,
    mqtt_msg_Subscribe,
    mqtt_msg_Publish,
    mqtt_msg_Ping,
    mqtt_msg_Disconnect,
} mqtt_msg_type_t;

/* Functions for handling acks */
bool Ack_Connect( uint8_t * buff, uint8_t len, uint16_t id );
bool Ack_Subscribe( uint8_t * buff, uint8_t len, uint16_t id );
bool Ack_Publish( uint8_t * buff, uint8_t len, uint16_t id );
bool Ack_Ping( uint8_t * buff, uint8_t len, uint16_t id );
bool Ack_Disconnect( uint8_t * buff, uint8_t len, uint16_t id );

typedef struct mqtt_pairs_t
{
    mqtt_msg_type_t msg_type;
    uint8_t send_code;
    uint8_t recv_code;
    bool (*ack_fn)(uint8_t * buff, uint8_t len, uint16_t id);
    const uint8_t * name;
    const uint8_t * ack_name;
} mqtt_pairs_t;

/* transmit and acknowledge code pairs */
/*
#static mqtt_pairs_t msg_code [ 5 ] =
{
    { mqtt_msg_Connect,         0x10, 0x20, Ack_Connect, "CONNECT", "CONNACK" },
    { mqtt_msg_Subscribe,       0x82, 0x90, Ack_Subscribe, "SUBSCRIBE", "SUBACK" },
    { mqtt_msg_Publish,         0x32, 0x40, Ack_Publish, "PUBLISH", "PUBACK" },
    { mqtt_msg_Ping,            0xc0, 0xd0, Ack_Ping, "PINGREQ", "PINGRESP" },
    { mqtt_msg_Disconnect,      0x00, 0x00, Ack_Disconnect, "DISCONNECT", "DISCONNECT" },
};
*/

static bool Transmit( mqtt_msg_type_t msg_type, void * msg_data )
{
    memset(send_buffer, 0x00, BUFFER_SIZE);
   
    bool ret = false;
    static uint16_t msg_id = 0x0000;
    /* Construct Data Packet */ 
//    uint16_t packet_size = MQTT_Format( msg_type, msg_data, &msg_id );
    uint16_t packet_size = 0U;

    /* Send */
    int snd = send(sock, send_buffer, packet_size, 0);
    if( snd < 0 )
    {
        printf("Error Sending Data\n");
        ret = false;
    }
    else
    {
        ret = true;
    }

    return ret;
}

extern bool MQTT_Connect( void )
{
    bool ret = false;
    
    int status;
    struct addrinfo hints, l_hints;
    struct addrinfo *servinfo, *l_servinfo;

    memset( &hints, 0U, sizeof( hints ) );
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    /* 1. Resolve port information */
    status = getaddrinfo( broker_ip, broker_port, &hints, &servinfo );
    if( status != 0U )
    {
        fprintf( stderr, "Comms Error: %s\n", gai_strerror( status ) );	
        ret = false;
    }
    else
    {   
        /* 2. Create Socket */
        sock = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if( sock < 0 )
        {
            printf("Error creating socket\n");
            ret = false;
        }
        else
        {
            /* 3. Attempt Connection */
            int c = connect( sock, servinfo->ai_addr, servinfo->ai_addrlen);
            if( c < 0 )
            {
                printf("Connection Failed\n");
                ret = false;
            }
            else
            {
                /* 4. Send Connect MQTT Packet */ 
  //              ret = Transmit( mqtt_msg_Connect, NULL );
                  ret = true;
            }
        }
    } 
    return ret;
}

extern void MQTT_Init( char * ip )
{
    assert( ip != NULL );
    printf("[MQTT]: Initialising\n");
    
    broker_ip = ip;
    broker_port = MQTT_PORT;
}

