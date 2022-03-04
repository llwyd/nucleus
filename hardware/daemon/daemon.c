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
void Daemon_TransmitLive(void);
void Daemon_TransmitVersion(void);
void Daemon_OnBoardLED( mqtt_data_t * data);

static task_t taskList[5] = 
{
	{ Sensor_ReadTemperature, 		1,		0},
	{ Aux_Update, 					1,		0},
	{ Daemon_TransmitLive, 			1, 		0},
	{ Daemon_TransmitTemperature, 	300, 	0},
	{ Daemon_TransmitVersion, 		3600, 	0},
};

static mqtt_subs_t subs[1] = 
{
	{"onboard_led", mqtt_type_bool, Daemon_OnBoardLED},
};

void Daemon_TransmitVersion(void)
{
	mqtt_pub_t version, location;

	version.name = "daemon_version";
	version.format = mqtt_type_str;
	version.data.s = (char *)daemonVersion;
	printf("PUBLISH->daemon_version->%s->", version.data.s);
	bool success = MQTT_Transmit(mqtt_msg_Publish, &version);
}

void Daemon_TransmitLive(void)
{
	mqtt_pub_t  inside;
	mqtt_pub_t outside;

	inside.name = "inside_temp_live";
	inside.format = mqtt_type_float;
	inside.data.f = Sensor_GetTemperature();
	
	printf("SENSOR->inside_temp_live->%.2foC->", inside.data.f);
	bool success = MQTT_Transmit(mqtt_msg_Publish, &inside);
}

void Daemon_TransmitTemperature(void)
{
	mqtt_pub_t  inside;

	inside.name = "inside_temp";
	inside.format = mqtt_type_float;
	inside.data.f = Sensor_GetTemperature();
	
	printf("SENSOR->inside_temp->%.2foC->", inside.data.f);
	bool success = MQTT_Transmit(mqtt_msg_Publish, &inside);
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
	Task_Init(taskList, 5);
	MQTT_Init(ip,port,"pi-livingroom","home/livingroom",subs,1);
	MQTT_Loop();
	return 0;
}

