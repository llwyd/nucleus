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
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum state_t
{
	state_ReadTemp,
	state_ReadWeather,
	state_SendData,
	state_RcvData,
	/*---------*/
	state_Count,
} state_t;


typedef struct state_data_t
{
	uint8_t *ip;		/* IP to transmit data */
	uint8_t *port;		/* Port number */
	
	/* Sensor Data*/
	float temperature;	/* Sensor Temperature */
	float humidity;		/* Sensor Humidity */

	/* Weather Description */
	uint8_t * weather;
} state_data_t;

typedef struct
{
	state_t currentState;					/* Current State Enumeration */	
	void (*state_fn)( state_data_t *data);	/* fn pointer */
	double time;							/* how often to execute */
	time_t currentTime;
} state_func_t;

/* State Prototypes */
void State_Init( state_data_t * data );
void State_ReadTemp( state_data_t * data );
void State_ReadWeather( state_data_t * data );
void State_SendData( state_data_t * data );
void State_RcvData( state_data_t * data );

state_func_t StateTable[] = 
{
	{state_ReadTemp,	State_ReadTemp, 	5,	0},
	{state_ReadWeather,	State_ReadWeather,	5,	0},
	{state_SendData,	State_SendData,		5,	0},
	{state_RcvData,		State_RcvData,		1,	0},
};

void State_Init( state_data_t * data )
{
	printf("Initial State\n");
}
void State_ReadTemp( state_data_t * data )
{
	printf("ReadTemp State\n");
}
void State_ReadWeather( state_data_t * data )
{
	printf("ReadWeather State\n");
}
void State_SendData( state_data_t * data )
{
	printf("SendData State\n");
}
void State_RcvData( state_data_t * data )
{
	printf("RcvData State\n");
}

uint8_t main( int16_t argc, uint8_t **argv )
{
	/* Data Structure for passing data between states */
	static state_data_t s;

	/* Master time struct for running states at set periods*/
	time_t stateTime;

	
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
		}
	}

	return 0;
}
