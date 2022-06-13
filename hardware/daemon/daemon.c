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
        default:
            printf("[Idle] Default Signal\n");
            ret = fsm_Ignored;
            break;
    }

    return ret;
}


static void Loop( void )
{
    fsm_t daemon; 
    daemon.state = &Daemon_Idle;    
    
    FSM_Init( &daemon );

    /* Get Event */

    /* Dispatch */

}

uint8_t main( int16_t argc, uint8_t **argv )
{
    Loop();
    return 0U;
}

