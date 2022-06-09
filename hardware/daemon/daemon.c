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


static void Init( void )
{

}

uint8_t main( int16_t argc, uint8_t **argv )
{
    Init();
    FSM_Dispatch( NULL, 0 );

    return 0U;
}

