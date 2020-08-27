/*
*	state.c
*
*	T.L. 2020
*/

#include "apikey.h"
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


void State_ReadDatabaseSize( state_data_t * data )
{
	struct stat st;
	stat( data->db, &st );

	data->databaseSize = st.st_size;

	printf( "Database Size: %d Bytes\n", data->databaseSize );
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

	printf("");
}

void State_RcvData( state_data_t * data )
{
	printf("");
}

