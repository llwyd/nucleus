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
	state_ReadAuxData,
	state_ReadCPUTemp,
	state_ToggleLED,
	state_SendData,
	state_RcvData,
	/*---------*/
	state_Count,
} state_t;


typedef struct state_data_t
{
	/* Network Transmission Info */
	uint8_t *ip;				/* IP to transmit data */
	uint8_t *port;				/* Port number */
	
	/* Sensor Data*/
	float temperature;			/* Sensor Temperature */
	float humidity;				/* Sensor Humidity */

	/* Weather Description */
	float outsideTemperature; 	/* Outside Temperature */
	float outsideHumidity;    	/* Outside Humidity */
	uint8_t * weather;			/* Outside weather description */

	/* Device Analytics */
	uint8_t * db;				/* Database Path */ 
	int databaseSize;			/* SQL database size */
	float cpuTemperature;		/* CPU Temperature */

	int uptimeDays;				/* Uptime Days */
	int uptimeHours;			/* Uptime Hours */
	int uptimeMinutes;			/* Uptime Minutes */
} state_data_t;

typedef struct
{
	state_t currentState;					/* Current State Enumeration */	
	void (*state_fn)( state_data_t *data);	/* fn pointer */
	double time;							/* how often to execute */
	time_t currentTime;						/* Keep track of last time task was executed */
} state_func_t;

/* State Prototypes */

extern state_func_t * State_GetTaskList( void );

void State_ReadTemp( state_data_t * data );
void State_ReadWeather( state_data_t * data );
void State_ReadCPUTemp( state_data_t * data );

void State_ToggleLED( state_data_t * data );
void State_ReadAuxData( state_data_t * data );

void State_SendAuxData( state_data_t * data);
void State_SendData( state_data_t * data );
void State_RcvData( state_data_t * data );

#endif /* STATE_H */

