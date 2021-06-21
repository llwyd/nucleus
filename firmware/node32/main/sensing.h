#ifndef _SENSING_H_
#define _SENSING_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

void Sensing_Init( QueueHandle_t * xTemperature );
void Sensing_Read( void );
extern void Sensing_Task( void * pvParameters );
const float * Sensing_GetTemperature( void );

#endif /* _SENSING_H_ */
