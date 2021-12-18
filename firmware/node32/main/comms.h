#ifndef _COMMS_H_
#define _COMMS_H_

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_http_client.h"

extern esp_err_t Comms_HTTPEventHandler( esp_http_client_event_t *evt );
extern void Comms_Task( void * pvParameters );
extern bool Comms_WifiConnected( void );

#endif /* _COMMS_H_ */
