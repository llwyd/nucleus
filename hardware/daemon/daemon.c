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
#include "mqtt.h"
#include "sensor.h"

#define NUM_SUBS ( 1U )

enum Signals
{
    signal_Tick = signal_Count,
    signal_MQTT_RECV,
    signal_Heartbeat,
    signal_UpdateHomepage,
};

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

fsm_status_t Daemon_Connect( fsm_t * this, signal s );
fsm_status_t Daemon_Subscribe( fsm_t * this, signal s );
fsm_status_t Daemon_Idle( fsm_t * this, signal s );

fsm_status_t Daemon_Connect( fsm_t * this, signal s )
{
    fsm_status_t ret = fsm_Ignored;
    switch( s )
    {
        case signal_Enter:
        case signal_Tick:
            printf("[Connect] Enter Signal\n");
            
            if( MQTT_Connect() )
            {
                mqtt_poll.fd = mqtt_sock;
                mqtt_poll.events = POLLIN;
                ret = fsm_Handled;
            }
            else
            {
                ret = fsm_Handled;
            }

            break;
        case signal_MQTT_RECV:
            printf("[Connect] MQTT_RECV Signal\n");
            
            if( MQTT_Receive() )
            {
                ret = fsm_Transition;
                this->state = &Daemon_Subscribe;
            }
            else
            {
                ret = fsm_Handled;
            }

            break;
        case signal_Exit:
            printf("[Connect] Exit Signal\n");
            ret = fsm_Handled;
            break;
        case signal_Heartbeat:
            printf("[Connect] Heartbeat Signal\n");
            Heartbeat();
            ret = fsm_Handled;
            break;
        case signal_None:
            assert(false);
            break;
        default:
            printf("[Connect] Default Signal\n");
            ret = fsm_Ignored;
            break;
    }

    return ret;
    
}

fsm_status_t Daemon_Subscribe( fsm_t * this, signal s )
{
    fsm_status_t ret = fsm_Ignored;
    switch( s )
    {
        case signal_Enter:
            printf("[Subscribe] Enter Signal\n");
            if( MQTT_Subscribe() )
            {
                ret = fsm_Handled;
            }
            else
            {
                ret = fsm_Transition;
                this->state = &Daemon_Connect;
            }
            break;
        case signal_Exit:
            printf("[Subscribe] Exit Signal\n");
            ret = fsm_Handled;
            break;
        case signal_MQTT_RECV:
            printf("[Subscribe] MQTT_RECV Signal\n");
            
            if( MQTT_Receive() )
            {
                if( MQTT_AllSubscribed() )
                {
                    ret = fsm_Transition;
                    this->state = &Daemon_Idle;
                }
                else
                {
                    ret = fsm_Handled;
                }
            }
            else
            {
                ret = fsm_Transition;
                this->state = &Daemon_Connect;
            }

            break;
        case signal_Heartbeat:
            printf("[Subscribe] Heartbeat Signal\n");
            Heartbeat();
            ret = fsm_Handled;
            break;
        case signal_None:
            assert(false);
            break;
        default:
        case signal_Tick:
            printf("[Subscribe] Default Signal\n");
            ret = fsm_Ignored;
            break;
    }

    return ret;
    
}

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
            {
                printf("[Idle] Tick Signal\n");
                Sensor_Read();
                float temperature = Sensor_GetTemperature();
                if( MQTT_EncodeAndPublish("temp_live", mqtt_type_float, &temperature ) )
                {
                    ret = fsm_Handled;
                }
                else
                {
                    ret = fsm_Transition;
                    this->state = &Daemon_Connect;
                }
            }
            break;
        case signal_UpdateHomepage:
            {
                printf("[Idle] Update Homepage Signal\n");
                float temperature = Sensor_GetTemperature();
                if( MQTT_EncodeAndPublish("node_temp", mqtt_type_float, &temperature ) )
                {
                    ret = fsm_Handled;
                }
                else
                {
                    ret = fsm_Transition;
                    this->state = &Daemon_Connect;
                }
            }
            break;
        case signal_MQTT_RECV:
            printf("[Idle] MQTT_RECV Signal\n");
            
            if( MQTT_Receive() )
            {
                ret = fsm_Handled;
            }
            else
            {
                ret = fsm_Transition;
                this->state = &Daemon_Connect;
            }

            break;
        case signal_Heartbeat:
            printf("[Idle] Heartbeat Signal\n");
            Heartbeat();
            ret = fsm_Handled;
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

void RefreshEvents( fsm_events_t * events )
{

    /* 500ms onboard blink */
    static struct timespec current_nano_tick;
    static struct timespec last_nano_tick;

    timespec_get( &current_nano_tick, TIME_UTC );
    if( ( current_nano_tick.tv_nsec - last_nano_tick.tv_nsec ) > 500000000UL )
    {
        last_nano_tick = current_nano_tick;
        FSM_AddEvent( events, signal_Heartbeat );
    }


    /* Check for MQTT/Comms events */
    int rv = poll( &mqtt_poll, 1, 1 );

    if( rv & POLLIN )
    {
        FSM_AddEvent( events, signal_MQTT_RECV );
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
        FSM_AddEvent( events, signal_Tick );
        last_tick = current_time;
    }

    delta = difftime( current_time, last_homepage_tick );
    if( delta > homepage_period )
    {
        FSM_AddEvent( events, signal_UpdateHomepage );
        last_homepage_tick = current_time;
    }

}

static void Loop( void )
{
    fsm_t daemon; 
    daemon.state = &Daemon_Connect; 
    signal sig = signal_None;
    fsm_events_t events;

    FSM_Init( &daemon, &events );

    while( 1 )
    {
        /* Get Event */
        
        while( !FSM_EventsAvailable( &events ) )
        {
            RefreshEvents( &events );
        }

        sig = FSM_GetLatestEvent( &events );

        /* Dispatch */
        FSM_Dispatch( &daemon, sig );
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

