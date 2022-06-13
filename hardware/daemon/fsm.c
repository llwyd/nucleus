#include "fsm.h"

extern void FSM_Init( fsm_t * state )
{
    FSM_Dispatch( state, signal_Enter );
}

extern void FSM_Dispatch( fsm_t * state, signal s )
{
    state_func previous = state->state;

    fsm_status_t status = state->state( state, s );

    if ( status == fsm_Transition )
    {
        previous( state, signal_Exit );
        state->state( state, signal_Enter );
    }
}

