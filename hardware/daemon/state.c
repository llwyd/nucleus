/*
*	state.c
*
*	T.L. 2020
*/

#include "state.h"
#include "../common/sensor.h"
#include <stdio.h>

void State_ReadTemp( state_data_t * data )
{
	Sensor_Read( &data->temperature );
	printf("Temperature: %.2f\n", data->temperature);
}

void State_ReadWeather( state_data_t * data )
{
	printf("");
}

void State_SendData( state_data_t * data )
{
	printf("");
}

void State_RcvData( state_data_t * data )
{
	printf("");
}

