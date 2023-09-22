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
#include "eeprom.h"
#include "cli.h"

DEFINE_STATE(Config);
DEFINE_STATE(AwaitingCommand);

typedef struct
{
    state_t state;
    uint8_t * buffer;
}
config_state_t;

#define NUM_COMMANDS (1)
const cli_command_t command_table[NUM_COMMANDS] =
{
    { "test", STATE(AwaitingCommand) },
};

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

static state_ret_t State_AwaitingCommand( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = PARENT(this, Config);
    config_state_t * config_state = (config_state_t *)this;
    switch(s)
    {
        case EVENT( Enter ):
        {
            CLI_Start();
            ret = HANDLED();
            break;
        }
        case EVENT( Exit ):
        {
            ret = HANDLED();
            break;
        }
        case EVENT( HandleCommand ):
        {
            CLI_Stop();
            bool valid_command = false;
            uint32_t idx;
            for( idx = 0; idx < NUM_COMMANDS; idx++)
            {
                if(strncmp(command_table[idx].command, config_state->buffer, CLI_CMD_SIZE) == 0U)
                {
                    valid_command = true;
                    break;
                } 
            }

            if(valid_command)
            {
                printf("Valid Command\n");
                CLI_Start();
                ret = HANDLED();

                /* TODO: Fix in state machine lib */
                //ret = TRANSITION2(this, command_table[idx].state);
            }
            else
            {
                printf("Invalid Command\n");
                CLI_Start();
                ret = HANDLED();
            }
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
    uint8_t command_buffer[CLI_CMD_SIZE];
    event_fifo_t events;
    critical_section_t crit;
    critical_section_t crit_events;
    critical_section_init_with_lock_num(&crit_events, 0U);
    
    config_state_t state_machine;
    state_machine.buffer = command_buffer;

    I2C_Init();
    CLI_Init(state_machine.buffer);
    Events_Init(&events);
    Emitter_Init(&events, &crit_events);
    STATEMACHINE_Init( &state_machine.state, STATE( AwaitingCommand ) );

    while(true)
    {
        while( FIFO_IsEmpty( (fifo_base_t *)&events ) )
        {
            tight_loop_contents();
        }
        critical_section_enter_blocking(&crit_events);
        event_t e = FIFO_Dequeue( &events );
        critical_section_exit(&crit_events);
        STATEMACHINE_Dispatch(&state_machine.state, e);
    }
    
    /* Shouldn't get here! */
    assert(false);
}
