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



static uint8_t * const ip 	= "0.0.0.0";
static uint8_t * const port	= "80";

static uint8_t * const databasePath = "../../homepage/app.db";

static state_func_t * task;

uint8_t main( int16_t argc, uint8_t **argv )
{
	/* Data Structure for passing data between states */
	static state_data_t s;

	task = State_GetTaskList();

	s.ip = ip;
	s.port = port;

	s.db = databasePath;

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
			double delta = difftime(stateTime, task[i].currentTime );
			
			if( delta > task[i].time )
			{
				task[i].state_fn( &s );
				task[i].currentTime = stateTime;
			}
			/* Delay to stop CPU melting */
			nanosleep(&delay, NULL);
		}
	}

	return 0;
}

