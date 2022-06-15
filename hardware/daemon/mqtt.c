#include "mqtt.h"

#include <time.h>

#define MQTT_PORT ( 1883 )
#define BUFFER_SIZE (128)

#define MQTT_TIMEOUT ( 0xb4 )

static int sock;
static struct pollfd mqtt_poll;

static uint8_t send_buffer[ BUFFER_SIZE ];
static uint8_t recv_buffer[ BUFFER_SIZE ];

static uint8_t * client_name;
static uint8_t * parent_topic;

static uint8_t num_sub = 0;

static uint8_t * broker_ip;
static uint8_t * broker_port;

extern void MQTT_Init( void )
{
    printf("[MQTT]: Initialising\n");
}

