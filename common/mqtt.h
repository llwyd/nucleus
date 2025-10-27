#ifndef MQTT_H_
#define MQTT_H_

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <string.h>
#include "pq_base.h"

#ifndef MQTT_MSG_SIZE
#define MQTT_MSG_SIZE (128u)
#endif

#ifndef MQTT_BUFFER_LEN
#define MQTT_BUFFER_LEN (64u)
#endif

#ifndef MQTT_TIMEOUT
#define MQTT_TIMEOUT ( 0xb4 )
#endif

typedef enum mqtt_msg_type_t
{
    MQTT_NONE = 0,
    MQTT_CONNECT,
    MQTT_CONNACK,
    MQTT_PUBLISH,
    MQTT_PUBACK,
    MQTT_PUBREC,
    MQTT_PUBCOMP,
    MQTT_SUBSCRIBE,
    MQTT_SUBACK,
    MQTT_UNSUB,
    MQTT_UNSUBACK,
    MQTT_PINGREQ,
    MQTT_SUBSRESP,
    MQTT_DISCONNECT,
    MQTT_AUTH,
} mqtt_msg_type_t;

typedef enum
{
    MQTT_OK = 0,
    MQTT_ERR,
    MQTT_SEND_ACK,
    MQTT_DISC,
}
mqtt_status_t;

typedef struct mqtt_sub_t
{
    char * name;
    bool global;
    bool (*callback_fn) (uint8_t *);
} mqtt_sub_t;

typedef struct mqtt_subs_t
{
    mqtt_sub_t * subs;
    uint32_t num_subs;
} mqtt_subs_t;

typedef struct
{
    pq_key_t key;
    uint8_t msg[MQTT_MSG_SIZE];
    uint16_t seq_num;
    uint16_t size;
    mqtt_msg_type_t type;
    uint32_t timestamp;
}
mqtt_msg_t;

typedef struct
{
   uint32_t subs;
   uint16_t seq_num;
   mqtt_status_t status;
   uint16_t rx_seq_num;
}
mqtt_state_t;

typedef struct
{
    uint8_t qos;
    bool global;
    uint8_t * topic;
    uint32_t timestamp;
    uint16_t seq_num;
}
mqtt_msg_params_t;

typedef struct
{
    bool (*fn) (uint8_t * message);
}
mqtt_callback_t;
typedef struct
{
    char * client_name;
    uint32_t timeout_ms;
    mqtt_subs_t * subs;
    pq_t * pq;
    mqtt_state_t state;
}
mqtt_t;

typedef enum
{
    MQTTPacket_Connect,
    MQTTPacket_Subscribe,
    MQTTPacket_Publish,
}
mqtt_packet_t;

/* Rewrite */
extern void MQTT_Init( mqtt_t * mqtt,
                char * client_name,
                mqtt_subs_t * subs,
                pq_t * pq,
                mqtt_msg_t * pool
                );

/* Return pointer instead */
extern mqtt_msg_t * MQTT_Encode(mqtt_t * mqtt,
        mqtt_msg_type_t type,
        uint8_t * data_buffer, 
        uint16_t data_len,
        mqtt_msg_params_t * params
        );

extern bool MQTT_Decode( mqtt_t * mqtt, uint8_t * buffer, uint16_t len);

extern mqtt_status_t MQTT_GetStatus(mqtt_t * mqtt);
extern uint32_t MQTT_CheckTimeouts(mqtt_t * mqtt);
extern mqtt_msg_t * MQTT_Peek(mqtt_t * mqtt, uint32_t index);
extern bool MQTT_AllSubscribed( mqtt_t * mqtt );

#endif /* MQTT_H_ */
