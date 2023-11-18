#ifndef COMMS_H_
#define COMMS_H_

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "msg_fifo.h"

extern char * Comms_GetLatestMessage(void);
void Comms_Init(char * ip, char * port, msg_fifo_t * fifo);
bool Comms_Connect(void);
extern bool Comms_MessageReceived(void);
bool Comms_Send(uint8_t * buffer, uint16_t len);
bool Comms_Recv(uint8_t * buffer, uint16_t len);
bool Comms_RecvToFifo(void);
extern bool Comms_Disconnected(void);

#endif /* COMMS_H_ */
