#include "udp.h"
#include "lwip/pbuf.h"

#define BUFFER_SIZE (256)

static uint8_t send_buffer[ BUFFER_SIZE ];
static uint8_t recv_buffer[ BUFFER_SIZE ];

static msg_fifo_t * msg_fifo;
static critical_section_t * critical;
static struct udp_pcb * udp_pcb;

static void Close(void)
{
    udp_remove(udp_pcb);
}

extern void UDP_Recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{

    Close();
}

extern bool UDP_Send( uint8_t * buffer, uint16_t len, ip_addr_t ip, uint16_t port)
{
    bool ret = true;
    if( udp_pcb != NULL )
    {
        Close();
    }

    udp_pcb = udp_new();
    struct pbuf * p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    memcpy(p->payload, buffer, len);
    
    critical_section_enter_blocking(critical);
    cyw43_arch_lwip_begin();
    err_t err = udp_sendto(udp_pcb, p,&ip,port);
    cyw43_arch_lwip_end();
    critical_section_exit(critical);
    
    pbuf_free(p);
    if(err != ERR_OK)
    {
        ret = false;
        Close();
    }

    return ret;
}

extern void UDP_Init(msg_fifo_t * fifo, critical_section_t * crit)
{
    printf("Initialising UDP Comms\n");
    msg_fifo = fifo;
    critical = crit;
}

