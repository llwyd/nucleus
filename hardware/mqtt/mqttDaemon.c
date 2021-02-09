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


void main( void )
{
    MQTT_Task();
}

