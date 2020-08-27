/*
*
*	Daemon.c
*
*	T.L. 2020
*
*/

#include <errno.h>
#include <inttypes.h>
#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


static state_func_t StateTable[] = 
{
	{state_ReadTemp,	State_ReadTemp, 	5,	0},
	{state_ReadWeather,	State_ReadWeather,	10,	0},
	{state_SendData,	State_SendData,		5,	0},
	{state_RcvData,		State_RcvData,		0,	0},
};

static uint8_t * const ip 	= "0.0.0.0";
static uint8_t * const port = "80";

uint8_t main( int16_t argc, uint8_t **argv )
{
	/* Data Structure for passing data between states */
	static state_data_t s;

	s.ip = ip;
	s.port = port;

	/* Master time struct for running states at set periods*/
	time_t stateTime;

	/* 100ms delay to prevent cpu melting */
	struct timespec delay;
	delay.tv_sec = 0;
	delay.tv_nsec = 100000000;
	
	/* Inf Loop */
	while (1)
	{
		/* Iterate through all the states sequentially */
		for( int i=0; i < state_Count; i++)
		{			
			time(&stateTime);
			double delta = difftime(stateTime, StateTable[i].currentTime );
			
			if( delta > StateTable[i].time )
			{
				StateTable[i].state_fn( &s );
				StateTable[i].currentTime = stateTime;
			}
			/* Delay to stop CPU melting */
			nanosleep(&delay, NULL);
		}
	}

	return 0;
}

