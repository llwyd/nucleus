/*
*
*	Daemon.c
*
*	T.L. 2020
*
*/

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "apikey.h"
#include "../common/aux.h"
#include "../common/mqtt.h"
#include "../common/task.h"
#include "../common/sensor.h"
#include "../common/weather.h"
#include "version.h"

static uint8_t * const ip 	= "0.0.0.0";
static uint8_t * const port	= "1883";

static uint8_t * const databasePath = "../../homepage/app.db";

void Daemon_TransmitTemperature(void);
void Daemon_TransmitAux(void);
void Daemon_OnBoardLED( mqtt_data_t * data);

static task_t taskList[4] = 
{
	{ Sensor_ReadTemperature, 		1,		0},
	{ Aux_Update, 					1,		0},
	{ Weather_Update, 				300, 	0},
	{ Daemon_TransmitTemperature, 	2, 	0},
};

static mqtt_subs_t subs[1] = 
{
	{"onboard_led", mqtt_type_bool, Daemon_OnBoardLED},
};


void Daemon_TransmitTemperature(void)
{
	mqtt_pub_t  inside;
	mqtt_pub_t outside;

	inside.name = "inside_temp";
	inside.format = mqtt_type_float;
	inside.data.f = Sensor_GetTemperature();
	
	printf("SENSOR->inside_temp->%.2foC->", inside.data.f);
	bool success = MQTT_Transmit(mqtt_msg_Publish, &inside);
	
	outside.name = "outside_temp";
	outside.format = mqtt_type_float;
	outside.data.f = Weather_GetTemperature();
	
	printf("SENSOR->outside_temp->%.2foC->", outside.data.f);
	success &= MQTT_Transmit(mqtt_msg_Publish, &outside);

	outside.name = "outside_desc";
	outside.format = mqtt_type_str;
	outside.data.s = Weather_GetDescription(); 
	printf("SENSOR->outside_desc->%s->", outside.data.s);
	success &= MQTT_Transmit(mqtt_msg_Publish, &outside);
}

void Daemon_OnBoardLED( mqtt_data_t * data)
{	
	int led_fd = open( "/sys/class/leds/led0/brightness", O_WRONLY );

	if(data->b)
	{	
		write( led_fd, "1", 1 );
	}
	else
	{
		write( led_fd, "0", 1 );
	}
	close( led_fd );
}

uint8_t main( int16_t argc, uint8_t **argv )
{
	Aux_Init(databasePath);
	Weather_Init(weatherLocation, weatherKey);
	Task_Init(taskList,4);
	MQTT_Init(ip,port,"pi-livingroom","home/livingroom",subs,1);
	MQTT_Loop();
	return 0;
}

