/*
*	state.h
*
*	T.L. 2020
*/

#ifndef STATE_H
#define STATE_H

#include <errno.h>
#include <inttypes.h>
#include "state.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
	time_t currentTime;						/* Keep track of last time task was executed */
} state_func_t;

/* State Prototypes */
void State_ReadTemp( state_data_t * data );
void State_ReadWeather( state_data_t * data );
void State_SendData( state_data_t * data );
void State_RcvData( state_data_t * data );

#endif /* STATE_H */

