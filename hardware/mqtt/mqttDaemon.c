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

#include "../common/mqtt.h"
#include "../common/task.h"


void Test_Task(void);
void Test_Task2(void);

task_t TaskList [2] =
{
    {Test_Task, 2, 0},
    {Test_Task2, 10, 0},
};

void Test_Task2(void)
{
    printf("Task 2 executed\n");
}
void Test_Task(void)
{
    printf("Task executed\n");
}

void Sub_Led(mqtt_data_t * data)
{
    printf("led_status: %d->", data->b );
} 

static mqtt_subs_t subscriptions [2]=
{
    {"led", mqtt_type_bool, Sub_Led },
    {"switch", mqtt_type_bool, Sub_Led },
};

void main( void )
{
    Task_Init(TaskList, 2);
    MQTT_Init("pi-livingroom","livingroom",subscriptions,2);
    MQTT_Task();
}

