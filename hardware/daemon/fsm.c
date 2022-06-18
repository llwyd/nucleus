#include "fsm.h"

#define BUFFER_SIZE ( 32U )
#define ENTER_CRITICAL {}
#define EXIT_CRITICAL  {}

typedef struct
{
    unsigned char read_index;
    unsigned char write_index;
    unsigned char fill;
    signal event[ BUFFER_SIZE ];
} fsm_events_t;

static fsm_events_t fsm_event;

extern void FSM_Init( fsm_t * state )
{
    fsm_event.read_index = 0U;
    fsm_event.write_index = 0U;
    fsm_event.fill = 0U;

    FSM_Dispatch( state, signal_Enter );
}

extern void FSM_Dispatch( fsm_t * state, signal s )
{
    state_func previous = state->state;
    fsm_status_t status = state->state( state, s );

    while ( status == fsm_Transition )
    {
        previous( state, signal_Exit );
        previous = state->state;
        status = state->state( state, signal_Enter );
    }
}

extern void FSM_FlushEvents( void )
{
    if( fsm_event.fill > 0U )
    {
        ENTER_CRITICAL;
        fsm_event.read_index = fsm_event.write_index;
        fsm_event.fill = 0U;
        EXIT_CRITICAL;
    }
}

extern void FSM_AddEvent( signal s )
{
    if( fsm_event.fill < BUFFER_SIZE )
    {
        ENTER_CRITICAL;
        fsm_event.event[ fsm_event.write_index++ ] = s;
        fsm_event.fill++;
        fsm_event.write_index = ( fsm_event.write_index & ( BUFFER_SIZE - 1U ) );
        EXIT_CRITICAL;
    }
}

extern bool FSM_EventsAvailable( void )
{
    return ( fsm_event.fill > 0U );
}

extern signal FSM_GetLatestEvent( void )
{
    signal s;

    ENTER_CRITICAL;
    s = fsm_event.event[ fsm_event.read_index++ ];
    fsm_event.fill--;
    fsm_event.read_index = ( fsm_event.read_index & ( BUFFER_SIZE - 1U ) );
    EXIT_CRITICAL;

    return s;
}

