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
#include "tcp.h"
#include "mqtt.h"
#include "node_events.h"
#include "i2c.h"
#include "accelerometer.h"
#include "udp.h"
#include "ntp.h"
#include "gpio.h"
#include "alarm.h"
#include "eeprom.h"
#include "meta.h"
#include "watchdog.h"
#include "uptime.h"
#include "dns.h"

#define NODE_EVENT(X) ("home/evnt/" X)

#define DNS_RETRY_ATTEMPTS (5U)
#define RETRY_ATTEMPTS (10U)
#define RETRY_PERIOD_MS (1000u)
#define SENSOR_PERIOD_MS (250u)
#define ACK_TIMEOUT_MS (2500u)

#define PQ_RETRY_MS (250u)
#define PQ_TIMEOUT_US (5000000)

#define ID_STRING_SIZE ( 32U )
#define MSG_BUFFER_SIZE ( 128U )

#define MQTT_PORT ( 1883 )

GENERATE_EVENT_STRINGS( EVENTS );

/* Top level state */
DEFINE_STATE(SetupWIFI);
DEFINE_STATE(ConfigureRTC);
DEFINE_STATE(Setup);
DEFINE_STATE(Root);

/* Second level */
DEFINE_STATE(WifiConnected);
DEFINE_STATE(WifiNotConnected);
DEFINE_STATE(DNSRequest);
DEFINE_STATE(RequestNTP);
DEFINE_STATE(Idle);

/* Third level */
DEFINE_STATE(TCPNotConnected);
DEFINE_STATE(MQTTNotConnected);
DEFINE_STATE(MQTTSubscribing);

/* Inherit from state_t */
typedef struct
{
    state_t state;
    uint32_t retry_counter;
    uint32_t dns_attempts;
    struct repeating_timer * timer;
    struct repeating_timer * read_timer;
    struct repeating_timer * retry_timer;
    mqtt_t * mqtt;
    ip_addr_t addr;
    ntp_t * ntp;
    tcp_t * tcp;
    critical_section_t * crit;
    uint8_t * msg_buffer;
    uint8_t * broker_ip;
}
node_state_t;

static bool ResetRequest(uint8_t * data);
static bool SensorRequest(uint8_t * data);
static bool UptimeRequest(uint8_t * data);
static bool ResyncRequest(uint8_t * data);

static mqtt_sub_t subs[4] = 
{
    {   
        .name = "home/reset",
        .callback_fn = ResetRequest,
        .global = true
    },
    {   
        .name = "home/sensor_rst", 
        .callback_fn = SensorRequest, 
        .global = true
    },
    {   
        .name = "home/reqmeta",
        .callback_fn = UptimeRequest,
        .global = true
    },
    {   
        .name = "home/resync",
        .callback_fn = ResyncRequest,
        .global = true
    },
};

static bool ResetRequest(uint8_t * data)
{
    (void)data;
    printf("\tRESET REQUEST!\n");
#ifdef WATCHDOG_ENABLED
    while(1);
#endif
    return true;
}

static bool SensorRequest(uint8_t * data)
{
    (void)data;
    Enviro_Init();
    return true;
}

static bool UptimeRequest(uint8_t * data)
{
    (void)data;
    Emitter_EmitEvent(EVENT(UptimeRequest));
    return true;
}

static bool ResyncRequest(uint8_t * data)
{
    (void)data;
    Emitter_EmitEvent(EVENT(Resync));
    return true;
}

static state_ret_t Publish(node_state_t * state,
                            event_t e,
                            mqtt_msg_params_t * params,
                            bool retry)
{
    state_t * this = &(state->state);
    state_ret_t ret = HANDLED();
    
    mqtt_msg_t * out = MQTT_Encode(state->mqtt,
            MQTT_PUBLISH,
            state->msg_buffer, 
            strlen((char*)state->msg_buffer),
            params);
    
    if(out != NULL)
    {
        if(TCP_Send(state->tcp, out->msg, out->size))
        {
            /* If send was successful, then add the dup flag here
             * incase of retransmission
             */
            out->msg[0] |= (1 << 3); // Dup flag
            ret = HANDLED();
        }
        else
        {
            TCP_Close(state->tcp);
            ret = TRANSITION(this, STATE(TCPNotConnected));
        }
    }
    else
    {
        if(retry)
        {
            /* PQ Buffer is full, re-emit event to try again */
            Emitter_EmitEvent(e);
        }
        ret = HANDLED();
    }

    return ret;
}

static state_ret_t State_Setup( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = NO_PARENT(this);
    node_state_t * node_state = (node_state_t *)this;
    (void)node_state;
    switch(s)
    {
        case EVENT( Tick ):
        {
            WIFI_ToggleLed();
            ret = HANDLED();
            break;
        }
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

static state_ret_t State_SetupWIFI( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = NO_PARENT(this);
    node_state_t * node_state = (node_state_t *)this;
    (void)node_state;
    switch(s)
    {
        case EVENT( Tick ):
        {
            WIFI_ToggleLed();
            ret = HANDLED();
            break;
        }
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


static state_ret_t State_WifiConnected( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = PARENT(this,STATE(Setup));
    node_state_t * node_state = (node_state_t *)this;
    (void)node_state;
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

static state_ret_t State_WifiNotConnected( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = PARENT(this, STATE(SetupWIFI));
    node_state_t * node_state = (node_state_t *)this;

    switch(s)
    {
        case EVENT( Enter ):
        {
            WIFI_Teardown();
            WIFI_TryConnect();
            Emitter_Create(EVENT(WifiCheckStatus), node_state->retry_timer, RETRY_PERIOD_MS);
            ret = HANDLED();
            break;
        }
        case EVENT( WifiCheckStatus ):
        {
            if( WIFI_CheckStatus() )
            {
                Emitter_Destroy(node_state->retry_timer);
                //ret = TRANSITION(this, STATE(TCPNotConnected));
                // Get NTP upon wifi connection 
                ret = TRANSITION(this, STATE(DNSRequest));
            }
            else
            {
                WIFI_TryConnect();
                Emitter_Destroy(node_state->retry_timer);
                Emitter_Create(EVENT(WifiCheckStatus), node_state->retry_timer, RETRY_PERIOD_MS);
                ret = HANDLED();
            }
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

static state_ret_t State_TCPNotConnected( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = PARENT(this, STATE(WifiConnected));
    node_state_t * node_state = (node_state_t *)this;
    switch(s)
    {
        case EVENT(RetryCounterIncrement):
        {
            Emitter_Destroy(node_state->retry_timer);
            node_state->retry_counter++;
            if(node_state->retry_counter < RETRY_ATTEMPTS)
            {
                Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
            }
            else
            {
                Emitter_Create(EVENT(TCPRetryConnect), node_state->retry_timer, RETRY_PERIOD_MS);
                Emitter_EmitEvent(EVENT(TCPDisconnected));
            }
            ret = HANDLED();
            break;
        }
        case EVENT( TCPRetryConnect ):
        case EVENT( Enter ):
        {
            ret = HANDLED();
            Emitter_Destroy(node_state->retry_timer);
            node_state->retry_counter = 0U;
            if(WIFI_CheckTCPStatus())
            {
                if(TCP_Connect(node_state->tcp))
                {
                    Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
                }
                else
                {
                    Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
                }
            }
            else
            {
                    ret = TRANSITION(this, STATE(WifiNotConnected));
            }
            break;
        }
        case EVENT( TCPConnected ):
        {
            TCP_Flush(node_state->tcp);
            Emitter_Destroy(node_state->retry_timer);
            ret = TRANSITION(this, STATE(MQTTNotConnected));
            break;
        }
        case EVENT( PCBInvalid ):
        case EVENT( TCPDisconnected ):
        {
            TCP_Close(node_state->tcp);
            ret = HANDLED();
            break;
        }
        case EVENT( TCPReceived ):
        {
            /* Upon a successful connection it is possible to receive left overs from previous
             * session, so "receive them so they are emptied from the buffer
             */
            (void)TCP_Retrieve(node_state->tcp, node_state->msg_buffer, MSG_BUFFER_SIZE);
            ret = HANDLED();
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

static state_ret_t State_MQTTNotConnected( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = PARENT(this, STATE(WifiConnected));
    node_state_t * node_state = (node_state_t *)this;
    switch(s)
    {
        case EVENT( Exit ):
        {
            ret = HANDLED();
            break;
        }
        case EVENT( PCBInvalid ):
        case EVENT( TCPDisconnected ):
        {
            TCP_Close(node_state->tcp);
            ret = TRANSITION(this, STATE(TCPNotConnected));
            break;
        }
        case EVENT(RetryCounterIncrement):
        {
            Emitter_Destroy(node_state->retry_timer);
            node_state->retry_counter++;
            if(node_state->retry_counter < RETRY_ATTEMPTS)
            {
                Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
            }
            else
            {
                Emitter_Create(EVENT(MQTTRetryConnect), node_state->retry_timer, RETRY_PERIOD_MS);
            }
            ret = HANDLED();
            break;
        }
        case EVENT( MQTTRetryConnect ):
        case EVENT( Enter ):
        {
            Emitter_Destroy(node_state->retry_timer);
            node_state->retry_counter = 0U;
            mqtt_msg_t * out = MQTT_Encode(node_state->mqtt,
                    MQTT_CONNECT,
                    NULL,
                    0,
                    NULL);

            if(TCP_Send(node_state->tcp, out->msg, out->size))
            {
                Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
                ret = HANDLED();
            }
            else
            {
                TCP_Close(node_state->tcp);
                ret = TRANSITION(this, STATE(TCPNotConnected));
            }
            break;
        }
        case EVENT( TCPReceived ):
        {
            /* Presumably the buffer has a message... */
            uint16_t recv_len = TCP_Retrieve(node_state->tcp, node_state->msg_buffer, MSG_BUFFER_SIZE);
            assert(recv_len > 0);
            if(MQTT_Decode(node_state->mqtt, node_state->msg_buffer, MSG_BUFFER_SIZE))
            {
                ret = TRANSITION(this, STATE(MQTTSubscribing));
            }
            else
            {
                Emitter_Create(EVENT(MQTTRetryConnect), node_state->retry_timer, RETRY_PERIOD_MS);
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

static state_ret_t State_MQTTSubscribing( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = PARENT(this, STATE(WifiConnected));
    node_state_t * node_state = (node_state_t *)this;
    switch(s)
    {
        case EVENT( TCPReceived ):
        {
            uint16_t recv_len = TCP_Retrieve(node_state->tcp, node_state->msg_buffer, MSG_BUFFER_SIZE);
            assert(recv_len > 0);
            if(MQTT_Decode(node_state->mqtt, node_state->msg_buffer, MSG_BUFFER_SIZE))
            {
                if(MQTT_AllSubscribed(node_state->mqtt))
                {
                    Emitter_Destroy(node_state->retry_timer);
                    ret = TRANSITION(this, STATE(Idle));
                }
                else
                {
                    ret = HANDLED();
                }
            }
            else
            {
                /* Something gone wrong, reconnect to TCP */
                printf("\tError, not received expected packet\n");
                TCP_Close(node_state->tcp);
                ret = TRANSITION(this, STATE(TCPNotConnected));
            }
            break;
        }
        case EVENT( TCPDisconnected ):
        {
            TCP_Close(node_state->tcp);
            ret = TRANSITION(this, STATE(TCPNotConnected));
            break;
        }
        case EVENT(RetryCounterIncrement):
        {
            Emitter_Destroy(node_state->retry_timer);
            node_state->retry_counter++;
            if(node_state->retry_counter < RETRY_ATTEMPTS)
            {
                Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
            }
            else
            {
                Emitter_EmitEvent(EVENT(TCPDisconnected));
            }
            ret = HANDLED();
            break;
        }
        case EVENT( Exit ):
        {
            ret = HANDLED();
            break;
        }
        case EVENT( Enter ):
        {
            Emitter_Destroy(node_state->retry_timer);
            node_state->retry_counter = 0U;
            bool success = true;
            for(uint32_t idx = 0; idx < node_state->mqtt->subs->num_subs; idx++)
            {
                /* TODO -> func for translating global and local topics */
                uint8_t * sub_topic = (uint8_t*)node_state->mqtt->subs->subs[idx].name;
                mqtt_msg_t * out = MQTT_Encode(node_state->mqtt, 
                                                    MQTT_SUBSCRIBE, 
                                                    sub_topic, 
                                                    strlen((char*)sub_topic), 
                                                    NULL);
                success &= TCP_Send(node_state->tcp, out->msg, out->size);
            }
            if(success)
            {
                Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
                ret = HANDLED();
            }
            else
            {
                TCP_Close(node_state->tcp);
                ret = TRANSITION(this, STATE(TCPNotConnected));
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

static state_ret_t State_ConfigureRTC( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = NO_PARENT(this);
    node_state_t * node_state = (node_state_t *)this;
    (void)node_state;
    switch(s)
    {
        case EVENT( Tick ):
        {
            ret = HANDLED();
            break;
        }
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

static state_ret_t State_DNSRequest( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = PARENT(this, STATE(ConfigureRTC));
    node_state_t * node_state = (node_state_t *)this;
    switch(s)
    {
        case EVENT(RetryCounterIncrement):
        {
            Emitter_Destroy(node_state->retry_timer);
            ret = HANDLED();
            
            node_state->retry_counter++;
            if(node_state->retry_counter < RETRY_ATTEMPTS)
            {
                Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
            }
            else
            {
                node_state->dns_attempts++;
                if(node_state->dns_attempts < DNS_RETRY_ATTEMPTS)
                {
                    Emitter_Create(EVENT(DNSRetryRequest), node_state->retry_timer, RETRY_PERIOD_MS);
                }
                else
                {
                    ret = TRANSITION(this, STATE(WifiNotConnected));
                }
            }
            break;
        }
        case EVENT( DNSRetryRequest ):
        case EVENT( Enter ):
        {
            ret = HANDLED();
            Emitter_Destroy(node_state->retry_timer);
            node_state->retry_counter = 0U;
            DNS_Request("pool.ntp.org");
            if(WIFI_CheckStatus())
            {
                Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
            }
            else
            {
                /* Possible WIFI may have failed at this point, re-connect */
                ret = TRANSITION(this, STATE(WifiNotConnected));
            }
            break;
        }
        case EVENT( DNSReceived ):
        {
            Emitter_Destroy(node_state->retry_timer);
            DNS_PrintIP();
            node_state->addr = DNS_Get();
            ret = TRANSITION(this, STATE(RequestNTP));
            break;
        }
        case EVENT( Exit ):
        {
            node_state->dns_attempts = 0u;
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

static state_ret_t State_RequestNTP( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = PARENT(this, STATE(ConfigureRTC));
    node_state_t * node_state = (node_state_t *)this;
    switch(s)
    {
        case EVENT(RetryCounterIncrement):
        {
            ret = HANDLED();
            Emitter_Destroy(node_state->retry_timer);
            node_state->retry_counter++;
            if(node_state->retry_counter < RETRY_ATTEMPTS)
            {
                Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
            }
            else
            {
                ret = TRANSITION(this, STATE(DNSRequest));
                //Emitter_Create(EVENT(NTPRetryRequest), node_state->retry_timer, RETRY_PERIOD_MS);
            }
            break;
        }
        case EVENT( NTPRetryRequest ):
        case EVENT( Enter ):
        {
            ret = HANDLED();
            Emitter_Destroy(node_state->retry_timer);
            NTP_Encode(node_state->msg_buffer);
            UDP_Send(node_state->msg_buffer, 
                    48U,
                    node_state->addr,
                    NTP_PORT,
                    node_state->crit);
            node_state->retry_counter = 0U;
            if(WIFI_CheckStatus())
            {
                Emitter_Create(EVENT(RetryCounterIncrement), node_state->retry_timer, RETRY_PERIOD_MS);
            }
            else
            {
                /* Possible WIFI may have failed at this point, re-connect */
                ret = TRANSITION(this, STATE(WifiNotConnected));
            }
            break;
        }
        case EVENT( Exit ):
        {
            ret = HANDLED();
            break;
        }
        case EVENT(UDPReceived):
        {
            Emitter_Destroy(node_state->retry_timer);
            UDP_Retrieve(node_state->msg_buffer, MSG_BUFFER_SIZE); 
            NTP_Decode(node_state->msg_buffer, node_state->ntp);
            NTP_Print(node_state->ntp);
            Alarm_SetClock(&node_state->ntp->transmit);
            TCP_Close(node_state->tcp);
            ret = TRANSITION(this, STATE(TCPNotConnected));
        }
        default:
        {
            break;
        }
    }
    return ret;
}

static state_ret_t State_Root( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = NO_PARENT(this);
    node_state_t * node_state = (node_state_t *)this;
    switch(s)
    {
        case EVENT( Enter ):
        {
            Emitter_Create(EVENT(ReadSensor), node_state->read_timer, SENSOR_PERIOD_MS);
            WIFI_SetLed();
            Alarm_Start();
            Accelerometer_Ack();
            Accelerometer_Start();
            ret = HANDLED();
            break;
        }
        case EVENT( Exit ):
        {
            /* Should never try and leave here! */
            WIFI_ClearLed();
            Alarm_Stop();
            Emitter_Destroy(node_state->read_timer);
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

static state_ret_t State_Idle( state_t * this, event_t s )
{
    STATE_DEBUG(s);
    state_ret_t ret = PARENT(this, STATE(Root));
    node_state_t * node_state = (node_state_t *)this;
    uint32_t timestamp = time_us_32();
    switch(s)
    {
        case EVENT( Exit ):
        {
            Emitter_Destroy(node_state->retry_timer);
            Emitter_Destroy(node_state->timer);
            ret = HANDLED();
            break;
        }
        case EVENT( Enter ):
        {
            Enviro_Init();
            Emitter_Create(EVENT(AckTimeout), node_state->retry_timer, ACK_TIMEOUT_MS);
            Emitter_Create(EVENT(PQResend), node_state->timer, PQ_RETRY_MS);
            ret = HANDLED();
            break;
        }
        case EVENT( AccelMotion ):
        {
            Accelerometer_Ack();
            Alarm_EncodeUnixTime((char*)node_state->msg_buffer, MSG_BUFFER_SIZE);
            mqtt_msg_params_t params =
            {
                .qos = 1,
                .global = false,
                .topic = (uint8_t*)NODE_EVENT("accl"),
                .timestamp = timestamp,
            };
            ret = Publish(node_state, s, &params, true);
            break;
        }
        case EVENT( GPIOAEvent ):
        {
            Alarm_EncodeUnixTime((char*)node_state->msg_buffer, MSG_BUFFER_SIZE);
            mqtt_msg_params_t params =
            {
                .qos = 1,
                .timestamp = timestamp,
                .global = false,
                .topic = (uint8_t*)NODE_EVENT("gpioa"),
            };
            ret = Publish(node_state, s, &params, true);
            break;
        }
        case EVENT( GPIOBEvent ):
        {
            Alarm_EncodeUnixTime((char*)node_state->msg_buffer, MSG_BUFFER_SIZE);
            mqtt_msg_params_t params =
            {
                .qos = 1,
                .timestamp = timestamp,
                .global = false,
                .topic = (uint8_t*)NODE_EVENT("gpiob"),
            };
            ret = Publish(node_state, s, &params, true);
            break;
        }
        case EVENT( UptimeRequest ):
        {
            Uptime_Encode((char*)node_state->msg_buffer, MSG_BUFFER_SIZE);
            printf("\tMeta: %s\n", node_state->msg_buffer);
            mqtt_msg_params_t params =
            {
                .qos = 1,
                .timestamp = timestamp,
                .global = false,
                .topic = (uint8_t*)"home/meta",
            };
            ret = Publish(node_state, s, &params, true);
            break;
        }
        case EVENT( PCBInvalid ):
        case EVENT( TCPDisconnected ):
        {
            TCP_Close(node_state->tcp);
            ret = TRANSITION(this, STATE(TCPNotConnected));
            break;
        }
        case EVENT( ReadSensor ):
        {
            Enviro_Read();
            Enviro_Print();

            Enviro_GenShortDigest((char*)node_state->msg_buffer, MSG_BUFFER_SIZE);
            mqtt_msg_params_t params =
            {
                .qos = 0,
                .timestamp = timestamp,
                .global = false,
                .topic = (uint8_t*)"home/env",
            };
            ret = Publish(node_state, s, &params, false);
            break;
        }
        case EVENT( TCPReceived ):
        {
            /* Presumably the buffer has a message... */
            uint16_t recv_len = TCP_Retrieve(node_state->tcp, node_state->msg_buffer, MSG_BUFFER_SIZE);
            assert(recv_len > 0);
            if(MQTT_Decode(node_state->mqtt, node_state->msg_buffer, MSG_BUFFER_SIZE))
            {
                /* May need to send ACK */
                ret = HANDLED();
                mqtt_status_t status = MQTT_GetStatus(node_state->mqtt);
                switch(status)
                {
                    case MQTT_SEND_ACK:
                    {
                        mqtt_msg_t * out = MQTT_Encode(node_state->mqtt,
                                MQTT_PUBACK, 
                                NULL, 
                                0u,
                                NULL);
                        /* Doesnt store in PQ so should never be null */
                        assert(out != NULL);
                        if(TCP_Send(node_state->tcp, out->msg, out->size))
                        {
                            ret = HANDLED();
                        }
                        else
                        {
                            TCP_Close(node_state->tcp);
                            ret = TRANSITION(this, STATE(TCPNotConnected));
                        }
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
            else
            {
                /* Something gone wrong, reconnect to TCP */
                printf("\tError, not received expected packet\n");
                TCP_Close(node_state->tcp);
                ret = TRANSITION(this, STATE(TCPNotConnected));
            }
            break;
        }
        case EVENT( AlarmElapsed ):
        {
            Enviro_GenDigest((char*)node_state->msg_buffer, MSG_BUFFER_SIZE);
            mqtt_msg_params_t params =
            {
                .qos = 1,
                .timestamp = timestamp,
                .global = false,
                .topic = (uint8_t*)"home/digest",
            };
            ret = Publish(node_state, s, &params, true);
            break;
        }
        case EVENT( PQResend ):
        {
            Emitter_Destroy(node_state->timer);
            Emitter_Create(EVENT(PQResend), node_state->timer, PQ_RETRY_MS);
            const uint32_t fill = node_state->mqtt->pq->fill;
            ret = HANDLED();
            for(uint32_t idx = 0; idx < fill; idx++)
            {
                mqtt_msg_t * b = (mqtt_msg_t *)PQ_Peek(node_state->mqtt->pq, idx);
                uint32_t delta = (timestamp - b->timestamp);
                if(delta > PQ_TIMEOUT_US)
                {
                    printf("\tts: %lu\n", timestamp);
                    printf("\tb->ts: %lu\n", b->timestamp);
                    printf("\td: %lu\n", delta);
                    printf("\tMQTT: Retransmitting %u\n", b->seq_num);
                    b->timestamp = timestamp;
                    if(TCP_Send(node_state->tcp, b->msg, b->size))
                    {
                        ret = HANDLED();
                    }
                    else
                    {
                        break;
                        TCP_Close(node_state->tcp);
                        ret = TRANSITION(this, STATE(TCPNotConnected));
                    }
                }
            }
            break;
        }
        case EVENT( AckReceived ):
        {
            printf("\tTCP ACK Received\n");
            TCP_Kick(node_state->tcp);
            Emitter_Destroy(node_state->retry_timer);
            Emitter_Create(EVENT(AckTimeout), node_state->retry_timer, ACK_TIMEOUT_MS);
            ret = HANDLED();
            break;
        }
        case EVENT( AckTimeout ):
        {
            printf("\tTCP ACK Timeout\n");
            TCP_Close(node_state->tcp);
            ret = TRANSITION(this, STATE(TCPNotConnected));
            break;
        }
        case EVENT( Resync ):
        {
            printf("\tResync\n");
            TCP_Close(node_state->tcp);
            ret = TRANSITION(this, STATE(DNSRequest));
            break;
        }
        default:
        {
            break;
        }
    }

    return ret;
}

extern void Daemon_Run(void)
{
    uint8_t msg_buffer[MSG_BUFFER_SIZE] = {0};
    uint8_t unique_id[ID_STRING_SIZE]={0};
    uint8_t broker_ip[EEPROM_ENTRY_SIZE] = {0U};
    pico_get_unique_board_id_string((char*)unique_id, ID_STRING_SIZE);

    event_fifo_t events;
    critical_section_t crit;
    mqtt_msg_t pool[PQ_FULL_LEN];
    mqtt_subs_t all_subs =
    {
        .subs = subs,
        .num_subs = 4u,
    };
   
    mqtt_t mqtt;

    ntp_t ntp = {0U};
    tcp_t tcp = {0U};

    pq_t pq;
    struct repeating_timer timer;
    struct repeating_timer read_timer;
    struct repeating_timer retry_timer;
    
    critical_section_t crit_events;
    critical_section_t crit_tcp;
    critical_section_t crit_udp;
   
    /* Initialise various sub modules */ 
    critical_section_init(&crit);
    critical_section_init_with_lock_num(&crit_events, 0U);
    critical_section_init_with_lock_num(&crit_tcp, 1U);
    critical_section_init_with_lock_num(&crit_udp, 2U);
    
    Watchdog_Init();
    I2C_Init();
    Alarm_Init();
    Uptime_Init();
    Enviro_Init();
    Accelerometer_Init();
    Events_Init(&events);
    EEPROM_Read(unique_id, EEPROM_ENTRY_SIZE, EEPROM_NAME);
    EEPROM_Read(broker_ip, EEPROM_ENTRY_SIZE, EEPROM_IP);

    TCP_Init(&tcp, (char *)broker_ip, MQTT_PORT, &crit_tcp);

    Watchdog_Kick();
    MQTT_Init(&mqtt, 
            (char*)unique_id, 
            &all_subs, 
            &pq,
            pool);
    Emitter_Init(&events, &crit_events);
    WIFI_Init();

    node_state_t state_machine; 
    state_machine.retry_counter = 0U;
    state_machine.dns_attempts = 0U;
    state_machine.timer = &timer;
    state_machine.read_timer = &read_timer;
    state_machine.retry_timer = &retry_timer;
    state_machine.mqtt = &mqtt;
    state_machine.ntp = &ntp;
    state_machine.crit = &crit;
    state_machine.msg_buffer = msg_buffer;
    state_machine.tcp = &tcp;

    Watchdog_Kick();
    STATEMACHINE_Init( &state_machine.state, STATE( WifiNotConnected ) );

    while( true )
    {
        while( FIFO_IsEmpty( (fifo_base_t *)&events ) )
        {
            tight_loop_contents();
        }
        critical_section_enter_blocking(&crit_events);
        event_t e = FIFO_Dequeue( &events );
        critical_section_exit(&crit_events);
        STATEMACHINE_Dispatch(&state_machine.state, e);
        //cyw43_arch_poll();
        Watchdog_Kick();
    }

    /* Shouldn't get here! */
    assert(false);
}

