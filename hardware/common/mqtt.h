#ifndef MQTT_H
#define MQTT_H


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

typedef enum mqtt_msg_type_t
{
    mqtt_msg_Connect,
    mqtt_msg_Subscribe,
    mqtt_msg_Publish,
    mqtt_msg_Ping,
    mqtt_msg_Disconnect,
} mqtt_msg_type_t;

typedef struct mqtt_func_t
{
    mqtt_state_t state;
    mqtt_state_t (*mqtt_fn)(void);
} mqtt_func_t;


uint16_t MQTT_Format( mqtt_msg_type_t msg_type, uint8_t * buffer, void * msg_data );
void MQTT_Transmit( mqtt_msg_type_t msg_type, uint8_t * out, uint8_t * in, void * msg_data );

mqtt_state_t MQTT_Connect( void );
mqtt_state_t MQTT_Subscribe( void );
mqtt_state_t MQTT_Idle( void );
mqtt_state_t MQTT_Publish( void );
mqtt_state_t MQTT_Disconnect( void );
mqtt_state_t MQTT_Ping( void );

void MQTT_Task( void );


#endif /* MQTT_H */
