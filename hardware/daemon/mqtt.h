#ifndef _MQTT_H_
#define _MQTT_H_

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
    mqtt_type_bool,
} mqtt_type_t;

extern bool MQTT_Receive( void );
extern bool MQTT_EncodeAndPublish( char * name, mqtt_type_t format, void * data );
extern void MQTT_Init( char * ip, char * name, int *mqtt_sock );
extern bool MQTT_Connect( void );

#endif /* _MQTT_H_ */
