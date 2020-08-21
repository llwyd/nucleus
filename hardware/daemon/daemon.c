/*
*
*	Daemon.c
*
*	T.L. 2020
*
*/
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef enum state_t
{
	state_Initialise,
	state_ReadTemp,
	state_ReadWeather,
	state_Delay,
	state_SendData,
	state_ReceiveData,
	state_Exit,
} state_t;

typedef union data_t
{
	float f;
	int i;
	uint8_t * c;
} data_t;

typedef enum
{
	data_Float,
	data_int16,
	data_Char,
	data_NULL,
} data_type_t;

typedef struct state_data_t
{
	uint8_t *ip;		/* IP to transmit data */
	uint8_t *port;		/* Port number */
	
	/* Sensor Data*/
	float temperature;	/* Sensor Temperature */
	float humidity;		/* Sensor Humidity */
} state_data_t;


void (*state_fn)( state_data_t *data, float);

static time_t stateTime;

/* State Prototypes */
void State_Init( state_data_t * data, float delay);
void State_ReadTemp( state_data_t * data );
void State_ReadWeather( state_data_t * data );
void State_Delay( state_data_t * data );
void State_Exit( state_data_t * data, float delay );


void State_Init( state_data_t * data, float delay)
{
	time_t currentTime;
	time( &currentTime );

	float delta = difftime( currentTime, stateTime );

	if( delta > delay )
	{
		printf("Time Elapsed\n");
		time( &stateTime );
		state_fn = State_Exit;
	}
	else
	{
		state_fn = State_Init;
	}
}

void State_Exit( state_data_t *data, float delay)
{
	printf("FIN\n");
}


uint8_t main( int16_t argc, uint8_t **argv )
{
	state_data_t s;
	time( &stateTime );

	state_fn = State_Init;
	while(state_fn != State_Exit)
	{
		state_fn(&s, 5.0f);
	}
	

	return 0;
}
