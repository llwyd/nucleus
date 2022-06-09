/*
 *
 * FSM Engine
 *
 * T.L. 2022
 *
 */
#ifndef _FSM_H_
#define _FSM_H_

/* Signal to send events to a given state */
typedef int signal;

/* Default signals for state machine */
enum DefaultSignals
{
    signal_Init,
    signal_Entry,
    signal_Exit,
};

typedef enum
{
    fsm_Handled,
    fsm_Transition,
    fsm_Ignored,
} fsm_status_t;

/* Forward declaration so that function pointer with state can return itself */
typedef struct fsm_t fsm_t;

/* Function pointer that holds the state to execute */
typedef fsm_status_t ( *state_func ) ( fsm_t * this, signal s );

struct fsm_t
{
    state_func state;
} ;

/* Event Dispatcher */
extern void FSM_Dispatch( fsm_t * state, signal s );

#endif /* _FSM_H_ */
