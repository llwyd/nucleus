#include "mqtt.h"

#include <time.h>
#include <assert.h>

#define MQTT_PORT ( "1883" )
#define BUFFER_SIZE (128)

#define MQTT_TIMEOUT ( 0xb4 )

static int * sock;

static uint8_t send_buffer[ BUFFER_SIZE ];
static uint8_t recv_buffer[ BUFFER_SIZE ];

static uint8_t * client_name;
static uint8_t * parent_topic = "home";

static uint8_t num_sub = 0;

static uint8_t * broker_ip;
static uint8_t * broker_port;

#define MQTT_CONNACK_CODE ( 0x20 )

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

static mqtt_pairs_t msg_code [ 5 ] =
{
    { mqtt_msg_Connect,         0x10, MQTT_CONNACK_CODE, Ack_Connect, "CONNECT", "CONNACK" },
    { mqtt_msg_Subscribe,       0x82, 0x90, Ack_Subscribe, "SUBSCRIBE", "SUBACK" },
    { mqtt_msg_Publish,         0x32, 0x40, Ack_Publish, "PUBLISH", "PUBACK" },
    { mqtt_msg_Ping,            0xc0, 0xd0, Ack_Ping, "PINGREQ", "PINGRESP" },
    { mqtt_msg_Disconnect,      0x00, 0x00, Ack_Disconnect, "DISCONNECT", "DISCONNECT" },
};

bool Ack_Connect( uint8_t * buff, uint8_t len, uint16_t id )
{
    bool ret = false;
    
    /* First byte is reserved */
    buff++;
    
    if( *buff == 0x00 )
    {
        printf("[MQTT] CONNACK:OK\n");
        ret = true;
    }
    else
    {
        printf("[MQTT] CONNACK:FAIL\n");
        ret = false;
    }

    return ret;
};

bool Ack_Subscribe( uint8_t * buff, uint8_t len, uint16_t id )
{
};

bool Ack_Publish( uint8_t * buff, uint8_t len, uint16_t id )
{
};

bool Ack_Ping( uint8_t * buff, uint8_t len, uint16_t id )
{
};

bool Ack_Disconnect( uint8_t * buff, uint8_t len, uint16_t id )
{
};

static uint16_t send_msg_id = 0x0000;
static uint16_t recv_msg_id = 0x0000;

static uint16_t Format( mqtt_msg_type_t msg_type, void * msg_data, uint16_t * id )
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
                0x00, MQTT_TIMEOUT,                     /* Keep alive timeout */
            };
            
            uint8_t * msg_ptr = send_buffer;
            uint16_t packet_size = (uint16_t)sizeof( mqtt_template_connect );
            uint16_t name_size = strlen( client_name );

            memcpy( msg_ptr, mqtt_template_connect, packet_size );
            msg_ptr+= packet_size;
            msg_ptr++;
            *msg_ptr++ = (uint8_t)(name_size & 0xFF);
            memcpy(msg_ptr, client_name, name_size);
                
            uint16_t total_packet_size = packet_size + name_size;
            send_buffer[1] = (uint8_t)(total_packet_size&0xFF);
            full_packet_size = total_packet_size + 2;
        }
            break;
        default:
        {

        }
            break;
    }
    return full_packet_size;
}

static bool Transmit( mqtt_msg_type_t msg_type, void * msg_data )
{
    memset(send_buffer, 0x00, BUFFER_SIZE);
   
    bool ret = false;
    /* Construct Data Packet */ 
    uint16_t packet_size = Format( msg_type, msg_data, &send_msg_id );

    /* Send */
    int snd = send( *sock, send_buffer, packet_size, 0);
    if( snd < 0 )
    {
        printf("[MQTT] Error Sending Data\n");
        ret = false;
    }
    else
    {
        printf("[MQTT] Transmitting %s packet\n", msg_code[(int)msg_type].name);
        ret = true;
    }

    return ret;
}

static bool Decode( uint8_t * buffer, uint16_t len )
{
    bool ret = false;
    uint8_t return_code = buffer[0];
    uint8_t msg_length = buffer[1];
    mqtt_msg_type_t msg_type;

    switch( return_code )
    {
        case MQTT_CONNACK_CODE:
            msg_type = mqtt_msg_Connect;
            break;
        default:
            printf("[MQTT] ERROR! Bad Receive Packet\n");
            assert( false );
            break;
    }
    
    printf("[MQTT] %s packet received, length: %d\n", msg_code[(int)msg_type ].name, msg_length );
    ret = msg_code[(int)msg_type].ack_fn( &buffer[2], msg_length, recv_msg_id );

    return ret;
}

extern bool MQTT_Receive( void )
{
    bool ret = false;

    int rcv = recv( *sock, recv_buffer, 256, 0U);
    if( rcv < 0 )
    {
        printf("[MQTT] Error Sending Data\n");
        ret = false;
    }
    else if( rcv == 0 )
    {
        printf("[MQTT] Connection Closed\n");
        ret = false;
    }
    else
    {
        printf("[MQTT] Data Received\n");

        ret = Decode( recv_buffer, rcv );

        printf("\t");
        for( int i = 0; i < rcv; i++ )
        {
            printf("0x%x,", recv_buffer[i]);
        }
        printf("\b \n");

        memset(recv_buffer, 0x00, BUFFER_SIZE);
    }
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
        *sock = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
        if( *sock < 0 )
        {
            printf("Error creating socket\n");
            ret = false;
        }
        else
        {
            /* 3. Attempt Connection */
            int c = connect( *sock, servinfo->ai_addr, servinfo->ai_addrlen);
            if( c < 0 )
            {
                printf("Connection Failed\n");
                ret = false;
            }
            else
            {
                /* 4. Send Connect MQTT Packet */ 
                ret = Transmit( mqtt_msg_Connect, NULL );
            }
        }
    } 
    return ret;
}

extern void MQTT_Init( char * ip, char * name, int *mqtt_sock )
{
    assert( ip != NULL );
    assert( mqtt_sock != NULL );
    assert( name != NULL );

    printf("[MQTT] Initialising\n");
    
    broker_ip = ip;
    broker_port = MQTT_PORT;
    client_name = name;
    sock = mqtt_sock;

    printf("[MQTT] Broker ip: %s, port: %s\n", broker_ip, broker_port);
    printf("[MQTT] Client name: %s\n", client_name );
}

