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

#include "state.h"
#include "mqtt.h"
#include "sensor.h"

#define NUM_SUBS ( 1U )

#define SIGNALS(SIG ) \
    SIG( MQTT_RECV ) \
    SIG( Heartbeat ) \
    SIG( UpdateHomepage ) \

GENERATE_SIGNALS( SIGNALS );
GENERATE_SIGNAL_STRINGS( SIGNALS );

static int mqtt_sock;
static struct pollfd mqtt_poll;
static char * broker_ip;
static char * client_name;
static int success_subs;

void Daemon_OnBoardLED( mqtt_data_t * data );
void Heartbeat( void );

static mqtt_subs_t subs[NUM_SUBS] = 
{
    {"debug_led", mqtt_type_bool, Daemon_OnBoardLED},
};

state_ret_t State_Connect( state_t * this, event_t s );
state_ret_t State_Subscribe( state_t * this, event_t s );
state_ret_t State_Idle( state_t * this, event_t s );

state_ret_t State_Connect( state_t * this, event_t s )
{
    STATE_DEBUG( s );
    state_ret_t ret;
    switch( s )
    {
        case EVENT( Enter ):
        case EVENT( Tick ):
            
            if( MQTT_Connect() )
            {
                mqtt_poll.fd = mqtt_sock;
                mqtt_poll.events = POLLIN;
            }
            else
            {
            }
            HANDLED();
            break;
        case EVENT( MQTT_RECV ):
            
            if( MQTT_Receive() )
            {
                TRANSITION( Subscribe );
            }
            else
            {
                HANDLED();
            }

            break;
        case EVENT( Exit ):
            HANDLED();
            break;
        case EVENT( Heartbeat ):
            Heartbeat();
            HANDLED();
            break;
        case EVENT( None ):
            assert(false);
            break;
        default:
            HANDLED();
            break;
    }

    return ret;
    
}

state_ret_t State_Subscribe( state_t * this, event_t s )
{
    STATE_DEBUG( s );
    state_ret_t ret;
    switch( s )
    {
        case EVENT( Enter ):
            if( MQTT_Subscribe() )
            {
                HANDLED();
            }
            else
            {
                TRANSITION( Connect );
            }
            break;
        case EVENT( Exit ):
            HANDLED();
            break;
        case EVENT( MQTT_RECV ):
            if( MQTT_Receive() )
            {
                if( MQTT_AllSubscribed() )
                {
                    TRANSITION( Idle );
                }
                else
                {
                    HANDLED();
                }
            }
            else
            {
                TRANSITION( Connect );
            }

            break;
        case EVENT( Heartbeat ):
            Heartbeat();
            HANDLED();
            break;
        case EVENT( None ):
            assert(false);
            break;
        default:
        case EVENT( Tick ):
            HANDLED();
            break;
    }

    return ret;
    
}

state_ret_t State_Idle( state_t * this, event_t s )
{
    STATE_DEBUG( s );
    state_ret_t ret;
    switch( s )
    {
        case EVENT( Enter ):
        case EVENT( Exit ):
            HANDLED();
            break;
        case EVENT( Tick ):
            {
                Sensor_Read();
                float temperature = Sensor_GetTemperature();
                if( MQTT_EncodeAndPublish("temp_live", mqtt_type_float, &temperature ) )
                {
                    HANDLED();
                }
                else
                {
                    TRANSITION( Connect );
                }
            }
            break;
        case EVENT( UpdateHomepage ):
            {
                float temperature = Sensor_GetTemperature();
                if( MQTT_EncodeAndPublish("node_temp", mqtt_type_float, &temperature ) )
                {
                    HANDLED();
                }
                else
                {
                    TRANSITION( Connect );
                }
            }
            break;
        case EVENT( MQTT_RECV ):
            
            if( MQTT_Receive() )
            {
                HANDLED();
            }
            else
            {
                TRANSITION( Connect );
            }

            break;
        case EVENT( Heartbeat ):
            Heartbeat();
            HANDLED();
            break;
        case EVENT( None ):
            assert(false);
            break;
        default:
            HANDLED();
            break;
    }

    return ret;
}

void RefreshEvents( state_fifo_t * events )
{

    /* 500ms onboard blink */
    static struct timespec current_nano_tick;
    static struct timespec last_nano_tick;

    timespec_get( &current_nano_tick, TIME_UTC );
    if( ( current_nano_tick.tv_nsec - last_nano_tick.tv_nsec ) > 500000000UL )
    {
        last_nano_tick = current_nano_tick;
        STATEMACHINE_AddEvent( events, EVENT( Heartbeat ) );
    }


    /* Check for MQTT/Comms events */
    int rv = poll( &mqtt_poll, 1, 1 );

    if( rv & POLLIN )
    {
        STATEMACHINE_AddEvent( events, EVENT( MQTT_RECV ) );
    }

    /* Check whether Tick has Elapsed */
    static time_t current_time;

    static time_t last_tick;
    static time_t last_homepage_tick;
    
    const double period = 1;
    const double homepage_period = 60;

    time( &current_time );

    double delta = difftime( current_time, last_tick );
    if( delta > period )
    {   
        STATEMACHINE_AddEvent( events, EVENT( Tick ) );
        last_tick = current_time;
    }

    delta = difftime( current_time, last_homepage_tick );
    if( delta > homepage_period )
    {
        STATEMACHINE_AddEvent( events, EVENT( UpdateHomepage ) );
        last_homepage_tick = current_time;
    }

}

static void Loop( void )
{
    state_t daemon; 
    daemon.state = State_Connect; 
    event_t sig = EVENT( None );
    state_fifo_t events;

    events.read_index = 0U;
    events.write_index = 0U;
    events.fill = 0U;

    STATEMACHINE_AddEvent( &events, EVENT( Enter ) );

    while( 1 )
    {
        /* Get Event */
        
        while( !STATEMACHINE_EventsAvailable( &events ) )
        {
            RefreshEvents( &events );
        }

        sig = STATEMACHINE_GetLatestEvent( &events );

        /* Dispatch */
        STATEMACHINE_FlatDispatch( &daemon, sig );
    }
}

void Daemon_OnBoardLED( mqtt_data_t * data )
{
    if( data->b )
    {
        printf("\tLED ON\n");
    }
    else
    {
        printf("\tLED OFF\n");
    }
}

void Heartbeat( void )
{
    int led_fd = open("/sys/class/leds/led0/brightness", O_WRONLY );
    static bool led_on;

    if( led_on )
    {
        write( led_fd, "1", 1 );
    }
    else
    {
        write( led_fd, "0", 1 );
    }
    close( led_fd );
    led_on ^= true;
}

bool Init( int argc, char ** argv )
{
    bool success = false;
    bool ip_found = false;
    bool name_found = false;
    int input_flags;

    while( ( input_flags = getopt( argc, argv, "b:c:" ) ) != -1U )
    {
        switch( input_flags )
        {
            case 'b':
                broker_ip = optarg;
                ip_found = true;
                break;
            case 'c':
                client_name = optarg;
                name_found = true;
                break;
            default:
                break;
        }
    } 
   
    success = ip_found & name_found;

    if( success )
    {
        MQTT_Init( broker_ip, client_name, &mqtt_sock, subs, NUM_SUBS );
    }

    return success;
}

uint8_t main( int argc, char ** argv )
{
    bool success = Init( argc, argv );

    if( success )
    {
        Loop();
    }
    else
    {
        printf("[CRITICAL ERROR!] FAILED TO INITIALISE\n");
    }
    return 0U;
}

