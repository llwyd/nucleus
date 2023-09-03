#include "udp.h"

#define BUFFER_SIZE (256)

static uint8_t send_buffer[ BUFFER_SIZE ];
static uint8_t recv_buffer[ BUFFER_SIZE ];

static msg_fifo_t * msg_fifo;
static critical_section_t * critical;

extern void UDP_Recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
}

extern bool UDP_Send( uint8_t * buffer, uint16_t len, char * ip, uint16_t port)
{
}

extern void UDP_Init(msg_fifo_t * fifo, critical_section_t * crit)
{
    printf("Initialising UDP Comms\n");
    msg_fifo = fifo;
    critical = crit;
}

