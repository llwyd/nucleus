/*
*
*	Home Automation Daemon
*
*	T.L. 2020 - 2022
*
*/

#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <assert.h>

#include "fsm.h"
#include "version.h"

enum Signals
{
    signal_Tick = signal_Count,
};

fsm_status_t Daemon_Idle( fsm_t * this, signal s )
{
    fsm_status_t ret = fsm_Ignored;
    switch( s )
    {
        case signal_Enter:
            printf("[Idle] Enter Signal\n");
            ret = fsm_Handled;
            break;
        case signal_Exit:
            printf("[Idle] Exit Signal\n");
            ret = fsm_Handled;
            break;
        case signal_Tick:
            printf("[Idle] Tick Signal\n");
            break;
        case signal_None:
            assert(false);
            break;
        default:
            printf("[Idle] Default Signal\n");
            ret = fsm_Ignored;
            break;
    }

    return ret;
}

signal GetEvent( void )
{
    static time_t current_time;
    static time_t last_tick;
    const double period = 1;

    signal ret = signal_None;

    time( &current_time );

    double delta = difftime( current_time, last_tick );

    if( delta > period )
    {   
        ret = signal_Tick;
        last_tick = current_time;
    }

    return ret;
}

static void Loop( void )
{
    fsm_t daemon; 
    daemon.state = &Daemon_Idle;    
    signal sig = signal_None;

    FSM_Init( &daemon );

    while( 1 )
    {
        /* Get Event */
        do
        {
            sig = GetEvent();
        }
        while( sig == signal_None );

        /* Dispatch */
        FSM_Dispatch( &daemon, sig );
    }
}

uint8_t main( int16_t argc, uint8_t **argv )
{
    Loop();
    return 0U;
}

