#ifndef _WEATHER_H_
#define _WEATHER_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "comms.h"

extern void Weather_Task( void * pvParameters );

#endif /* _WEATHER_H_ */

