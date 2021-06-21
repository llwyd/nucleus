#ifndef _COMMS_H_
#define _COMMS_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void Comms_Init( QueueHandle_t * xTemperature );
extern void Comms_Task( void * pvParameters );

#endif /* _COMMS_H_ */
