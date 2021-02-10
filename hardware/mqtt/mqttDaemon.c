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

//#include "localdata.h"
#include "../common/mqtt.h"



void Sub_Led(mqtt_data_t * data)
{
    printf("led_status: %s->", data->s );
} 

static mqtt_subs_t subscriptions [1]=
{
    {"led", mqtt_type_bool, Sub_Led },
};

void main( void )
{
    MQTT_Init("pi-livingroom","livingroom",subscriptions,1);
    MQTT_Task();
}

