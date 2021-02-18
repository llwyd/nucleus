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
#include "../common/mqtt.h"
#include "../common/task.h"
#include "../common/sensor.h"
#include "version.h"

static uint8_t * const ip 	= "0.0.0.0";
static uint8_t * const port	= "1883";

static uint8_t * const databasePath = "../../homepage/app.db";
static uint8_t weatherDesc[ 128 ];


void Daemon_ProcessTemperature(void);
void Daemon_SetLED( mqtt_data_t * data);

static task_t taskList[2] = 
{
	{ Sensor_ReadTemperature, 1, 0},
	{ Daemon_ProcessTemperature, 60, 0},
};

static mqtt_subs_t subs[1] = 
{
	{"led", mqtt_type_bool, Daemon_SetLED},
};


void Daemon_ProcessTemperature(void)
{
	mqtt_pub_t temperature;
	temperature.name = "temperature";
	temperature.format = mqtt_type_float;
	temperature.data.f = Sensor_GetTemperature();
	printf("SENSOR->TEMPERATURE->%.2foC->", temperature.data.f);
	bool success = MQTT_Transmit(mqtt_msg_Publish, &temperature);
}

void Daemon_SetLED( mqtt_data_t * data)
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
	Task_Init(taskList,2);
	MQTT_Init(ip,port,"pi-livingroom","livingroom",subs,1);
	MQTT_Loop();
	return 0;
}

