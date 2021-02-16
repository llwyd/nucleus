/* mqtt.c */

#include "../common/mqtt.h"
#include "../mqtt/localdata.h"
#include <time.h>

#define MQTT_PORT ( 1883 )
#define BUFFER_SIZE (128)

#define MQTT_TIMEOUT ( 0xb4 )

static int sock;
static struct pollfd mqtt_poll;

static uint8_t sendBuffer[BUFFER_SIZE];
static uint8_t recvBuffer[BUFFER_SIZE];

static uint8_t * client_name;
static uint8_t * parent_topic;

static mqtt_subs_t * sub;
static uint8_t num_sub = 0;

/* This is used to keep track of time between MQTT events */
static time_t watchdog;


static mqtt_func_t StateTable [ 3 ] =
{
    { mqtt_state_Connect ,      MQTT_Connect },
    { mqtt_state_Subscribe ,    MQTT_Subscribe },
    { mqtt_state_Idle,          MQTT_Idle },
};

/* Functions for handlinmg acks */
bool Ack_Connect( uint8_t * buff, uint8_t len, uint16_t id );
bool Ack_Subscribe( uint8_t * buff, uint8_t len, uint16_t id );
bool Ack_Publish( uint8_t * buff, uint8_t len, uint16_t id );
bool Ack_Ping( uint8_t * buff, uint8_t len, uint16_t id );
bool Ack_Disconnect( uint8_t * buff, uint8_t len, uint16_t id );

/* transmit and acknowledge code pairs */
static mqtt_pairs_t msg_code [ 5 ] =
{
    { mqtt_msg_Connect,         0x10, 0x20, Ack_Connect, "CONNECT", "CONNACK" },
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
        printf("OK");
        ret = true;
    }
    else
    {
        printf("FAIL");
        ret = false;
    }

    return ret;
}
bool Ack_Subscribe( uint8_t * buff, uint8_t len, uint16_t id )
{
    bool ret = false;
    
    uint16_t msg_id =  ( *buff++ << 8 ) | *buff++;
    uint8_t qos = *buff;

    printf("msg id=%d, qos = %d->", msg_id, qos);
    
    if(msg_id == id)
    {
        ret = true;
        printf("OK");
    }
    else
    {
        ret = false;
        printf("FAIL");
    }
    
    mqtt_poll.fd = sock;
    mqtt_poll.events = POLLIN;
    memset(recvBuffer, 0x00, 128);

    return ret;
}
bool Ack_Publish( uint8_t * buff, uint8_t len, uint16_t id )
{
    bool ret = false;
    uint16_t msg_id =  ( *buff++ << 8 ) | *buff++;
    printf("msg id=%d->", msg_id);
    
    if(msg_id == id)
    {
        ret = true;
        printf("OK");
    }
    else
    {
        ret = false;
        printf("FAIL");
    }

    return ret;
}
bool Ack_Ping( uint8_t * buff, uint8_t len, uint16_t id )
{
    printf("OK");
    return true;
}
bool Ack_Disconnect( uint8_t * buff, uint8_t len, uint16_t id )
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

    bool success = true;
    for( uint8_t i=0; i < num_sub; i++)
    {
        success &= MQTT_Transmit( mqtt_msg_Subscribe, &sub[i]);
    }

    if( success )
    {
        ret = mqtt_state_Idle;
    }

    return ret; 
}

mqtt_state_t MQTT_Ping( void )
{
    mqtt_state_t ret = mqtt_state_Connect;
    
    bool success = MQTT_Transmit( mqtt_msg_Ping, NULL);
    if( success )
    {
        ret = mqtt_state_Idle;
    }
    else
    {
        ret = mqtt_state_Connect;
    }
    
    return ret;
}

mqtt_data_t MQTT_Extract( uint8_t * data, mqtt_type_t type )
{
    mqtt_data_t d;
    
    switch( type )
    {
        case mqtt_type_float:
            d.f = (float)atof(data);
            break;
        case mqtt_type_int16:
            d.i = (uint16_t)atoi(data);
            break;
        case mqtt_type_str:
            d.s = data;
            break;
        case mqtt_type_bool:
            if( strcmp("1", data) == 0) 
            {
                d.b = true; 
            }
            else
            {
                d.b = false;
            }
            break;
        default:
            break;
    }
    return d;
}

/* Function for decoding unsolicited messages */
void MQTT_Decode( uint8_t * buffer, uint16_t len )
{
    printf("PUBLISH->recv:%d->", len);
    mqtt_data_t decode_data;
    if( buffer[0] == 0x30 )
    {
        uint8_t topic[64];
        uint8_t full_topic[64];
        uint8_t data[32];

        memset( topic, 0x00, 64);
        memset( data, 0x00, 32);
                
        unsigned char msgLen = buffer[1];
        unsigned char topLen = buffer[2] << 8 | buffer[3];
        unsigned char * msg = &buffer[4];
        
        memcpy(topic, msg, topLen);
        msg+=topLen;
        msgLen -= topLen;
        
        for( uint8_t i = 0; i < num_sub ; i++ )
        {
            memset(full_topic, 0x00, 64);
            
            strcat(full_topic, parent_topic);
            strcat(full_topic, "/");
            strcat(full_topic, sub[i].name);
            
            if( strcmp( full_topic, topic) == 0)
            {
                printf("%s->",topic);
                memcpy(data, msg, msgLen);
                printf("data:%s->",data);
                decode_data = MQTT_Extract( data, sub[i].format);
                sub[i].sub_fn( &decode_data );
                printf("OK\n");
            }
        }


    }
    else
    {
        printf("FAIL\n");
    }
}

bool MQTT_CheckTime()
{
    time_t currentTime;
    time(&currentTime);
    double mqtt_timeout = (double)MQTT_TIMEOUT;
    bool ret = false;
    double delta = difftime( currentTime, watchdog );
    if( delta > mqtt_timeout )
    {
        ret = true;
    }
    else
    {
        ret = false;
    }
    return ret;
}

void MQTT_KickWatchdog(void)
{
    time(&watchdog);
}

mqtt_state_t MQTT_Idle( void )
{
    mqtt_state_t ret = mqtt_state_Idle;
    bool ping = MQTT_CheckTime();    
    int rv = poll( &mqtt_poll, 1, 1);
    static uint8_t task_idx = 0;

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
            MQTT_Decode( recvBuffer, rcv );
            MQTT_KickWatchdog();
            memset(recvBuffer, 0x00, 128);
            ret = mqtt_state_Idle;
        }
    }
    else if( ping )
    {
        /*Ping watchdog has expired, send ping*/
        ret = MQTT_Ping();
    }
    else
    {
        /*Perform task */
        Task_RunSingle();
    }
    
    return ret;
}

uint16_t MQTT_Format( mqtt_msg_type_t msg_type, void * msg_data, uint16_t * id )
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
            mqtt_subs_t * sub_data = (mqtt_subs_t *) msg_data;
            const unsigned char mqtt_template_subscribe[] =
            {
                0x82,                                                                   // message type, qos, no retain
                0x00,                                                                   // length of message
            };
    
            uint8_t full_topic[64];
            memset( full_topic, 0x00, 64);
            
            strcat(full_topic, parent_topic);
            strcat(full_topic,"/");
            strcat(full_topic, sub_data->name);

            memset(sendBuffer, 0x00, 128);
            memset(recvBuffer, 0x00, 128);

            /* Construct Packet */
            uint8_t * msg_ptr = sendBuffer;
            uint16_t packet_size = (uint16_t)sizeof( mqtt_template_subscribe );
            uint16_t topic_size = strlen(full_topic );

            memcpy( msg_ptr, mqtt_template_subscribe, packet_size );
            msg_ptr+= packet_size;
            
            *msg_ptr++ = (uint8_t)((*id >> 8U)&0xFF);
            *msg_ptr++ = (uint8_t)(*id&0xFF);
            
            msg_ptr++;
            *msg_ptr++ = (uint8_t)(topic_size & 0xFF);
            memcpy(msg_ptr, full_topic, topic_size);
     
            uint16_t total_packet_size = packet_size + topic_size + 1 + 2;
            sendBuffer[1] = (uint8_t)(total_packet_size&0xFF);
            full_packet_size = total_packet_size + 2;
        }
            break;
        case mqtt_msg_Publish:
        {
            mqtt_pub_t * pub_data = (mqtt_pub_t *) msg_data;
            
            memset(sendBuffer, 0x00, 128);
            memset(recvBuffer, 0x00, 128);
            
            uint8_t text_buff[64];
            memset( text_buff, 0x00, 64);
            
            strcat(text_buff, parent_topic);
            strcat(text_buff,"/");
            strcat(text_buff, pub_data->name);
            uint16_t topic_size = strlen(text_buff);
            

            uint8_t * msg_ptr = sendBuffer;
            *msg_ptr++ = msg_code[mqtt_msg_Publish].send_code;
            msg_ptr++; /* This is where message length would go */ 
            msg_ptr++; /* MSB topic length */
            *msg_ptr++ = (uint8_t)(topic_size & 0xFF);
            memcpy(msg_ptr, text_buff, topic_size);
            memset( text_buff, 0x00, 64);
            msg_ptr+=topic_size;

            /* Message ID */
            *msg_ptr++ = (uint8_t)((*id >> 8U)&0xFF);
            *msg_ptr++ = (uint8_t)(*id&0xFF);
            switch( pub_data->format )
            {
                case mqtt_type_float:
                    snprintf(text_buff, 64, "%.4f", pub_data->data.f);
                    break;
                case mqtt_type_int16:
                    snprintf(text_buff, 64, "%df", pub_data->data.i);
                    break;
                case mqtt_type_str:
                    strcat(text_buff, pub_data->data.s);
                    break;
                case mqtt_type_bool:
                    if( pub_data->data.b)
                    {
                        strcat(text_buff,"1");
                    }
                    else
                    {
                        strcat(text_buff,"0");
                    }
                    break;
            }
            uint8_t data_len = strlen(text_buff);
            memcpy(msg_ptr,text_buff,data_len);
            /* Topic len + topic + data + msg id */
            uint8_t total_packet_size = (uint8_t)topic_size + data_len + 2 + 2;
            sendBuffer[1] = total_packet_size;
            /* header and size byte */
            full_packet_size = total_packet_size + 2;

        }
            break;
        case mqtt_msg_Ping:
        {
            const unsigned char mqtt_template_ping[] =
            {
                0xc0,0x00,
            };
            
            memset(sendBuffer, 0x00, 128);
            memset(recvBuffer, 0x00, 128);
            
            uint8_t * msg_ptr = sendBuffer;
            uint16_t packet_size = (uint16_t)sizeof( mqtt_template_ping );
            memcpy( msg_ptr, mqtt_template_ping, packet_size );
            full_packet_size = packet_size;
         }   
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
    static uint16_t msg_id = 0x0000;
    /* Construct Data Packet */ 
    uint16_t packet_size = MQTT_Format( msg_type, msg_data, &msg_id );
    
    /* Send */
    int snd = send(sock, sendBuffer, packet_size, 0);
    if( snd < 0 )
    {
        printf("Error Sending Data\n");
        ret = false;
    }
    else
    {
        printf("%s->", msg_code[msg_type].name);
        printf("sent:%d->", snd);    
        int rcv = recv(sock, recvBuffer, 128, 0);                
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
            printf("recv:%d->", rcv);    
            
            if( recvBuffer[0] == msg_code[msg_type].recv_code )
            {
                uint8_t msg_length = recvBuffer[1];
                printf("%s->", msg_code[msg_type].ack_name);
                printf("msglen:%d->", msg_length);
                ret = msg_code[msg_type].ack_fn( &recvBuffer[2], msg_length, msg_id );
                msg_id++;
                printf("\n");
            }
            else
            {
                printf("FAIL\n");
                ret = false;
            }
        }
    }

    MQTT_KickWatchdog();
    return ret;
}

void MQTT_Init( uint8_t * host_name, uint8_t * root_topic, mqtt_subs_t * subs, uint8_t num_subs)
{
    client_name         = host_name;
    parent_topic        = root_topic;
    sub = subs;
    num_sub = num_subs; 

    printf("INIT->host:%s->topic:%s->OK\n", client_name , parent_topic);
    for( uint8_t i=0; i < num_subs; i++ )
    {
        printf("INIT->sub:%s->OK\n", sub[i].name);
    }
    time(&watchdog);
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

