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

#define HTTP_BUFFER_SIZE ( 2048 )

static uint8_t httpSend[ HTTP_BUFFER_SIZE ];
static uint8_t httpRcv[ HTTP_BUFFER_SIZE ];

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

	printf("Outside Temperature: %.2f C\n", data->outsideTemperature );
	printf("   Outside Humidity: %.2f\%\n", data->outsideHumidity );
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

void State_RcvData( state_data_t * data )
{
	printf("");
}

