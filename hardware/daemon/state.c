/*
*	state.c
*
*	T.L. 2020
*/

#include "apikey.h"
#include <math.h>
#include "state.h"
#include "../common/sensor.h"
#include "../common/comms.h"
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <unistd.h>


#define HTTP_BUFFER_SIZE ( 2048 )

static uint8_t httpSend[ HTTP_BUFFER_SIZE ];
static uint8_t httpRcv[ HTTP_BUFFER_SIZE ];

static state_func_t StateTable[ 8 ] = 
{
	{state_ReadTemp,			State_ReadTemp, 		5,		0},
	{state_ReadWeather,			State_ReadWeather,		120,	0},
	{state_ReadCPUTemp,			State_ReadCPUTemp,		1,		0},
	{state_ToggleLED,			State_ToggleLED,		1,		0},
	{state_ReadAuxData,			State_ReadAuxData,		60,		0},
	{state_SendData,			State_SendData,			300,	0},
	{state_SendAuxData,			State_SendAuxData,		60,		0},
	{state_RcvData,				State_RcvData,			0,		0},
};

extern state_func_t * State_GetTaskList( void )
{
	return StateTable;
}

void State_ReadTemp( state_data_t * data )
{
	Sensor_Read( &data->temperature );
	data->humidity = 0.0f;

	printf("Temperature: %.2f C\n", data->temperature);
}

void State_ReadWeather( state_data_t * data )
{
	memset( httpSend, 0x00, HTTP_BUFFER_SIZE );
	memset(  httpRcv, 0x00, HTTP_BUFFER_SIZE );

	snprintf( httpSend,HTTP_BUFFER_SIZE,"/data/2.5/weather?q=%s&appid=%s",weatherLocation,weatherKey);

	Comms_Get( "api.openweathermap.org", "80", httpSend, httpRcv, HTTP_BUFFER_SIZE );
	Comms_ExtractTemp( httpRcv, &data->outsideTemperature );
	Comms_ExtractFloat( httpRcv, &data->outsideHumidity, "\"humidity\"" );

	Comms_ExtractValue( httpRcv, data->weather, "\"description\":");

	printf("Outside Temperature: %.2f C\n",	data->outsideTemperature );
	printf("   Outside Humidity: %.2f\%\n",	data->outsideHumidity );
	printf("        Description:  %s\n", 	data->weather );
}

void State_ReadCPUTemp( state_data_t * data )
{
	int fd = open("/sys/class/thermal/thermal_zone0/temp", O_RDONLY);
	char temp[4] = {0};

	read( fd, &temp,3);
	close( fd );	

	data->cpuTemperature = atof(temp);
	data->cpuTemperature/=10;

	printf("CPU Temp: %.1f C\n", data->cpuTemperature);
}

void State_ToggleLED( state_data_t * data )
{
	const uint8_t ledState[2]= {'0', '1'};
	static uint8_t idx = 0;
	
	int led_fd = open( "/sys/class/leds/led0/brightness", O_WRONLY );
	
	write( led_fd, &ledState[idx++], 1 );

	close( led_fd );

	idx %= 2;
}

void State_ReadAuxData( state_data_t * data )
{
	/* Database size on website */
	struct stat st;
	stat( data->db, &st );

	data->databaseSize = st.st_size;

	/* Server uptime */
	struct sysinfo info;
	sysinfo( &info );

	long uptime = info.uptime;
	data->uptimeDays = (int)floor((float)uptime / ( 24.f * 3600.f ));
	uptime -= ( data->uptimeDays * 3600 * 24) ;

	data->uptimeHours = (int)floor(((float)uptime / 3600.f));
	uptime -= ( data->uptimeHours * 3600);

	data->uptimeMinutes = (int)floor(((float) uptime / 60.f ));

	printf( "Database Size: %d Bytes\n", data->databaseSize );
	printf("%d Days, %d hours and %d minutes\n", data->uptimeDays, data->uptimeHours, data->uptimeMinutes);
}

void State_SendData( state_data_t * data )
{
	memset( httpSend, 0x00, HTTP_BUFFER_SIZE );
	memset(  httpRcv, 0x00, HTTP_BUFFER_SIZE );
	
	Comms_FormatTempData( deviceUUID, &data->temperature, &data->humidity, httpSend, HTTP_BUFFER_SIZE );
	Comms_Post( data->ip, data->port, "/raw", httpSend );

	memset( httpSend, 0x00, HTTP_BUFFER_SIZE );
	memset(  httpRcv, 0x00, HTTP_BUFFER_SIZE );
	
	Comms_FormatTempData( weatherUUID, &data->outsideTemperature, &data->outsideHumidity, httpSend, HTTP_BUFFER_SIZE );	
	Comms_Post( data->ip, data->port, "/raw", httpSend );

	printf("Transmitted data to cloud.\n");
}

void State_SendAuxData( state_data_t * data)
{
	json_data_t auxData[ 5 ];
	memset( httpSend, 0x00, HTTP_BUFFER_SIZE );
	memset(  httpRcv, 0x00, HTTP_BUFFER_SIZE );

	Comms_PopulateJSON( &auxData[0], "days",	(json_field_t *)&data->uptimeDays,		json_int );
	Comms_PopulateJSON( &auxData[1], "hours",	(json_field_t *)&data->uptimeHours,		json_int );
	Comms_PopulateJSON( &auxData[2], "minutes",	(json_field_t *)&data->uptimeMinutes,	json_int );
	
	Comms_PopulateJSON( &auxData[3], "database_size", (json_field_t *)&data->databaseSize, json_int );

	Comms_PopulateJSON( &auxData[4], "cpu_temp", (json_field_t *)&data->cpuTemperature, json_float );

	Comms_FormatData( httpSend, HTTP_BUFFER_SIZE, auxData,	5);
	Comms_Post( data->ip, data->port, "/stats", httpSend );

	printf("%s\n", httpSend );
}

void State_RcvData( state_data_t * data )
{
	//printf("");
}

