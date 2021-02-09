/* mqtt.c */

#include "../common/mqtt.h"
#include "../mqtt/localdata.h"

#define MQTT_PORT ( 1883 )
#define BUFFER_SIZE (128)

static int sock;
static struct pollfd mqtt_poll;

static uint8_t sendBuffer[BUFFER_SIZE];
static uint8_t recvBuffer[BUFFER_SIZE];

static const uint8_t * client_name      = "pi-livingroom";
static const uint8_t * parent_topic     = "livingroom";

static mqtt_func_t StateTable [ 3 ] =
{
    { mqtt_state_Connect ,      MQTT_Connect },
    { mqtt_state_Subscribe ,    MQTT_Subscribe },
    { mqtt_state_Idle,          MQTT_Idle },
    //{ mqtt_state_Publish ,      MQTT_Publish },
    //{ mqtt_state_Disconnect ,   MQTT_Disconnect },
    //{ mqtt_state_Ping ,         MQTT_Ping },
};

/* Functions for handlinmg acks */
bool Ack_Connect( uint8_t * buff, uint8_t len );
bool Ack_Subscribe( uint8_t * buff, uint8_t len );
bool Ack_Publish( uint8_t * buff, uint8_t len );
bool Ack_Ping( uint8_t * buff, uint8_t len );
bool Ack_Disconnect( uint8_t * buff, uint8_t len );

/* transmit and acknowledge code pairs */
static mqtt_pairs_t msg_code [ 5 ] =
{
    { mqtt_msg_Connect,         0x10, 0x20, Ack_Connect },
    { mqtt_msg_Subscribe,       0x82, 0x90, Ack_Subscribe },
    { mqtt_msg_Publish,         0x32, 0x40, Ack_Publish },
    { mqtt_msg_Ping,            0xc0, 0xd0, Ack_Ping },
    { mqtt_msg_Disconnect,      0x00, 0x00, Ack_Disconnect },
};


bool Ack_Connect( uint8_t * buff, uint8_t len )
{
    bool ret = false;
    
    /* First byte is reserved */
    buff++;
    
    if( *buff == 0x00 )
    {
        printf("CONNACK: Connection Accepted!\n");
        ret = true;
    }
    else
    {
        printf("CONNACK: Connection Refused\n");
        ret = false;
    }

    return ret;
}
bool Ack_Subscribe( uint8_t * buff, uint8_t len )
{
    bool ret = true;
    
    uint16_t msg_id =  ( *buff++ << 0 ) | *buff++;
    uint8_t qos = *buff;

    printf("SUBACK: msg id=%d, qos = %d\n", msg_id, qos);
    
    mqtt_poll.fd = sock;
    mqtt_poll.events = POLLIN;
    memset(recvBuffer, 0x00, 128);

    return ret;
}
bool Ack_Publish( uint8_t * buff, uint8_t len )
{
    return true;
}
bool Ack_Ping( uint8_t * buff, uint8_t len )
{
    return true;
}
bool Ack_Disconnect( uint8_t * buff, uint8_t len )
{
    return true;
}

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
                bool success = MQTT_Transmit( mqtt_msg_Connect, NULL);
                if( success )
                {
                    ret = mqtt_state_Subscribe;
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

    bool success = MQTT_Transmit( mqtt_msg_Subscribe, NULL);    
    if( success )
    {
        ret = mqtt_state_Idle;
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

uint16_t MQTT_Format( mqtt_msg_type_t msg_type, void * msg_data )
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
        {
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
            full_packet_size = total_packet_size + 2;
        }
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



bool MQTT_Transmit( mqtt_msg_type_t msg_type, void * msg_data )
{
    memset(sendBuffer, 0x00, BUFFER_SIZE);
    memset(recvBuffer, 0x00, BUFFER_SIZE);
   
    bool ret = false;
    /* Construct Data Packet */ 
    uint16_t packet_size = MQTT_Format( msg_type, NULL );
    
    /* Send */
    int snd = send(sock, sendBuffer, packet_size, 0);
    if( snd < 0 )
    {
        printf("Error Sending Data\n");
        ret = false;
    }
    else
    {
        printf("%d Bytes Sent\n", snd);    
        int rcv = recv(sock, recvBuffer, 128, 0);                
        if( rcv < 0 )
        {
            printf("Error Sending Data\n");
            ret = false;
        }
        else if( rcv == 0 )
        {
            printf("Connection Closed\n");
            ret = false;
        }
        else
        {
            printf("%d Bytes Received\n", rcv);    
            
            if( recvBuffer[0] == msg_code[msg_type].recv_code )
            {
                uint8_t msg_length = recvBuffer[1];
                printf("Correct ACK code received (0x%x)\n", recvBuffer[0]);
                printf("MQTT Message Length: 0x%x\n", msg_length);
                ret = msg_code[msg_type].ack_fn( &recvBuffer[2], msg_length );
            }
            else
            {
                printf("Incorrect ACK code received (0x%x)\n", recvBuffer[0]);
                ret = false;
            }
        }
    }

    return ret;
}

void MQTT_Task( void )
{
    mqtt_func_t * task = StateTable;
    mqtt_state_t current_state = mqtt_state_Connect;    

    while(1)
    {
        current_state = task[current_state].mqtt_fn();
    }
}

