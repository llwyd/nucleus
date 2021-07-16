#ifndef _SENSING_H_
#define _SENSING_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "types.h"

extern void Sensing_Task( void * pvParameters );

#endif /* _SENSING_H_ */
