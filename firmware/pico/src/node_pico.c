#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/critical_section.h"

#include "environment.h"
#include "events.h"
#include "fifo_base.h"
#include "state.h"
#include "wifi.h"
#include "emitter.h"

#define SIGNALS(SIG ) \
    SIG( Tick ) \
    SIG( ReadSensor ) \
    SIG( WifiCheckStatus ) \
    SIG( WifiConnected ) \
    SIG( WifiDisconnected ) \

GENERATE_SIGNALS( SIGNALS );
GENERATE_SIGNAL_STRINGS( SIGNALS );

DEFINE_STATE( Idle );

/* Inherit from state_t */
typedef struct
{
    state_t state;
    struct repeating_timer * timer;
    struct repeating_timer * read_timer;
    struct repeating_timer * wifi_timer;
}
node_state_t;

static state_ret_t State_Idle( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = NO_PARENT(this);
    node_state_t * node_state = (node_state_t *)this;

    switch(s)
    {
        case EVENT( Enter ):
        {
            WIFI_TryConnect();
            Emitter_Create(EVENT(Tick), node_state->timer, 500);
            Emitter_Create(EVENT(ReadSensor), node_state->read_timer, 1000);
            Emitter_Create(EVENT(WifiCheckStatus), node_state->wifi_timer, 5000);
            break;
        }
        case EVENT( ReadSensor ):
        {
            Enviro_Read();
            Enviro_Print();
            ret = HANDLED();
            break;
        }
        case EVENT( WifiCheckStatus ):
        {
            if( WIFI_CheckStatus() )
            {
                printf("\tWifi Connected! :)\n");
                Emitter_Destroy(node_state->wifi_timer);
                WIFI_SetLed();
                ret = HANDLED();
            }
            else
            {
                printf("\tWifi Disconnected! :(\n");
                WIFI_TryConnect();
                Emitter_Destroy(node_state->wifi_timer);
                Emitter_Create(EVENT(WifiCheckStatus), node_state->wifi_timer, 5000);
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

int main()
{
    event_fifo_t events;
    critical_section_t crit;
    
    struct repeating_timer timer;
    struct repeating_timer read_timer;
    struct repeating_timer wifi_timer;
    
    stdio_init_all();
    critical_section_init(&crit);
    Enviro_Init(); 
    Events_Init(&events);
    Emitter_Init(&events, &crit);
    (void)WIFI_Init();

    node_state_t state_machine; 
    state_machine.state.state = State_Idle; 
    state_machine.timer = &timer;
    state_machine.read_timer = &read_timer;
    state_machine.wifi_timer = &wifi_timer;
    
    FIFO_Enqueue( &events, EVENT(Enter) );

    while( true )
    {
        while( FIFO_IsEmpty( (fifo_base_t *)&events ) )
        {
            tight_loop_contents();
        }
        event_t e = FIFO_Dequeue( &events );
        STATEMACHINE_Dispatch(&state_machine.state, e);
    }
    return 0;
}
