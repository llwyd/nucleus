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

static event_fifo_t events;
static critical_section_t crit;

static bool Tick(struct repeating_timer *t)
{
    critical_section_enter_blocking(&crit);
    if( !FIFO_IsFull(&events.base) )
    {
        FIFO_Enqueue(&events, EVENT(Tick));
    }
    
    critical_section_exit(&crit);
    return true;
}

static bool ReadTimer(struct repeating_timer *t)
{
    critical_section_enter_blocking(&crit);
    if( !FIFO_IsFull(&events.base) )
    {
        FIFO_Enqueue(&events, EVENT(ReadSensor));
    }
    
    critical_section_exit(&crit);
    return true;
}

static bool CheckWifi(struct repeating_timer *t)
{
    int status = cyw43_wifi_link_status(&cyw43_state, CYW43_ITF_STA );
    if( status == CYW43_LINK_JOIN )
    {
        critical_section_enter_blocking(&crit);
        if( !FIFO_IsFull(&events.base) )
        {
            FIFO_Enqueue(&events, EVENT(WifiConnected));
        }
        critical_section_exit(&crit);
    }
    else if (status < 0 )
    {
        critical_section_enter_blocking(&crit);
        if( !FIFO_IsFull(&events.base) )
        {
            FIFO_Enqueue(&events, EVENT(WifiDisconnected));
        }
        critical_section_exit(&crit);
    }
    return true;
}

static void Blink(void)
{
    static bool state = true;
    //cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,(uint8_t)state);
    state^=true;
}

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
            add_repeating_timer_ms(500U, Tick, NULL, node_state->timer);
            add_repeating_timer_ms(1000U, ReadTimer, NULL, node_state->read_timer);
            add_repeating_timer_ms(5000U, CheckWifi, NULL, node_state->wifi_timer);
            break;
        }
        case EVENT( ReadSensor ):
        {
            Enviro_Read();
            Enviro_Print();
            ret = HANDLED();
            break;
        }
        case EVENT( Tick ):
        {
            Blink();
            ret = HANDLED();
            break;
        }
        case EVENT( WifiConnected ):
        {
            printf("Wifi Connected!\n");
            cancel_repeating_timer(node_state->wifi_timer);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1); 
            ret = HANDLED();
            break;
        }
        case EVENT( WifiDisconnected ):
        {
            printf("Wifi Not connected!\n");
            WIFI_TryConnect();
            cancel_repeating_timer(node_state->wifi_timer);
            add_repeating_timer_ms(5000U, CheckWifi, NULL, node_state->wifi_timer);
            ret = HANDLED();
            break;
        }
        case EVENT( Exit ):
        default:
        {
            break;
        }
    }
}

int main()
{
    struct repeating_timer timer;
    struct repeating_timer read_timer;
    struct repeating_timer wifi_timer;
    
    stdio_init_all();
    critical_section_init(&crit);

    printf("--------------------------\n");    
    Enviro_Init(); 
    Events_Init(&events);

    node_state_t state_machine; 
    state_machine.state.state = State_Idle; 
    state_machine.timer = &timer;
    state_machine.read_timer = &read_timer;
    state_machine.wifi_timer = &wifi_timer;
    
    FIFO_Enqueue( &events, EVENT(Enter) );

    (void)WIFI_Init();

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
