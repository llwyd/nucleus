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

#define SIGNALS(SIG ) \
    SIG( Tick ) \
    SIG( ReadSensor ) \

GENERATE_SIGNALS( SIGNALS );
GENERATE_SIGNAL_STRINGS( SIGNALS );

DEFINE_STATE( Idle );

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

static void Blink(void)
{
    static bool state = true;

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN,(uint8_t)state);

    state^=true;
}

static state_ret_t State_Idle( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = NO_PARENT(this);

    switch(s)
    {
        case EVENT( Enter ):
        case EVENT( Exit ):
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
        default:
        {
            break;
        }
    }
}

int main()
{
    stdio_init_all();
    cyw43_arch_init();
    critical_section_init(&crit);
    struct repeating_timer timer;
    struct repeating_timer read_timer;

    printf("--------------------------\n");    
    Enviro_Init(); 
    Events_Init(&events);
    
    state_t state_machine; 
    state_machine.state = State_Idle; 
    
    FIFO_Enqueue( &events, EVENT(Enter) );

    add_repeating_timer_ms(500U, Tick, NULL, &timer);
    add_repeating_timer_ms(1000U, ReadTimer, NULL, &read_timer);

    while( true )
    {
        while( FIFO_IsEmpty( (fifo_base_t *)&events ) )
        {
            tight_loop_contents();
        }
        event_t e = FIFO_Dequeue( &events );
        STATEMACHINE_Dispatch(&state_machine, e);
    }
    return 0;
}
