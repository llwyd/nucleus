#ifndef _COMMS_H_
#define _COMMS_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void Comms_Init( QueueHandle_t * xTemperature, QueueHandle_t * xSlowTemperature );
extern void Comms_Task( void * pvParameters );
extern void Comms_Weather( void * pvParameters );

#endif /* _COMMS_H_ */
