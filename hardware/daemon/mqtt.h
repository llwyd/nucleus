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

extern bool MQTT_Receive( void );
extern void MQTT_Init( char * ip, char * name, int *mqtt_sock );
extern bool MQTT_Connect( void );

#endif /* _MQTT_H_ */
