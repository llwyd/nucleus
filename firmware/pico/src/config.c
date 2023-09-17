#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/critical_section.h"
#include "pico/unique_id.h"

#include "daemon.h"
#include "environment.h"
#include "events.h"
#include "fifo_base.h"
#include "state.h"
#include "wifi.h"
#include "emitter.h"
#include "comms.h"
#include "mqtt.h"
#include "msg_fifo.h"
#include "node_events.h"
#include "i2c.h"
#include "accelerometer.h"
#include "udp.h"
#include "ntp.h"
#include "gpio.h"
#include "alarm.h"

DEFINE_STATE(Config);

typedef struct
{
    state_t state;
}
config_state_t;

static state_ret_t State_Config( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = NO_PARENT(this);
    config_state_t * config_state = (config_state_t *)this;
    switch(s)
    {
        case EVENT( Enter ):
        {
            ret = HANDLED();
            break;
        }
        case EVENT( Exit ):
        {
            ret = HANDLED();
            break;
        }
        default:
        {
            break;
        }
    }
    return ret;
}

extern void Config_Run(void)
{
    config_state_t state_machine; 
    STATEMACHINE_Init( &state_machine.state, STATE( Config ) );
    while(true)
    {

    }
    
    /* Shouldn't get here! */
    assert(false);
}
