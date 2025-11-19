#ifndef TCP_H_
#define TCP_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "fifo_base.h"
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "pico/critical_section.h"

#include "msg_fifo.h"

typedef struct
{
    struct tcp_pcb * pcb;
    ip_addr_t ip;
    uint16_t port;
    critical_section_t * crit;
    err_t err;
    /* Number of bytes in transit */
    uint32_t bytes;
}
tcp_t;

extern void TCP_Init(tcp_t * tcp, char * buffer, uint16_t port, critical_section_t * crit);
extern bool TCP_Connect(tcp_t * tcp);
extern bool TCP_Send(tcp_t * tcp, uint8_t * buffer, uint16_t len );
extern void TCP_Close(tcp_t * tcp);
extern void TCP_Abort(tcp_t * tcp);
extern void TCP_Flush(tcp_t * tcp);
extern uint16_t TCP_Retrieve(tcp_t * tcp, uint8_t * buffer, uint16_t len);
extern bool TCP_MemoryError(tcp_t * tcp);
extern void TCP_Kick(tcp_t * tcp);
extern void TCP_FreeBytes(tcp_t * tcp);

#endif /* TCP_H_ */
