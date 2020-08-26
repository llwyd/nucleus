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

#define HTTP_BUFFER_SIZE ( 2048 )

static uint8_t httpSend[ HTTP_BUFFER_SIZE ];
static uint8_t httpRcv[ HTTP_BUFFER_SIZE ];

void State_ReadTemp( state_data_t * data )
{
	Sensor_Read( &data->temperature );
	printf("Temperature: %.2f\n", data->temperature);
}

void State_ReadWeather( state_data_t * data )
{
	memset( httpSend, 0x00, HTTP_BUFFER_SIZE );
	memset(  httpRcv, 0x00, HTTP_BUFFER_SIZE );

	snprintf( httpSend,HTTP_BUFFER_SIZE,"/data/2.5/weather?q=%s&appid=%s",weatherLocation,weatherKey);

	Comms_Get( "api.openweathermap.org", "80", httpSend, httpRcv, HTTP_BUFFER_SIZE );
	Comms_ExtractTemp( httpRcv, &data->outsideTemperature );

	printf("%.2f\n", data->outsideTemperature );
}

void State_SendData( state_data_t * data )
{
	printf("");
}

void State_RcvData( state_data_t * data )
{
	printf("");
}

