#ifndef _MQTT_H_
#define _MQTT_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "dataqueue.h"
#include "mqtt_client.h"

extern esp_mqtt_client_handle_t * MQTT_GetClient( void );
extern bool MQTT_BrokerConnected( void );
extern void MQTT_ConstructPacket( dq_data_t * data, char * topic, uint16_t topicSize, char * dataBuffer, uint16_t dataBufferSize );
extern void MQTT_Init( void );

#endif /* _MQTT_H_ */
