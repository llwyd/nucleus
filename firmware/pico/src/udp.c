#include "udp.h"
#include "lwip/pbuf.h"
#include "emitter.h"
#include "node_events.h"

#define BUFFER_SIZE (128)

static struct udp_pcb * udp_pcb;

/* TODO: Replace with FIFO */
static struct pbuf * p_holder;

static void Close(void)
{
    udp_remove(udp_pcb);
}

extern void Recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port)
{
    (void)arg;
    (void)pcb;
    (void)addr;
    (void)port;
    cyw43_arch_lwip_check();
    assert(p->tot_len < BUFFER_SIZE); 

    if( p != NULL )
    {
        uint16_t pbufs_recvd = pbuf_clen( p ); 
        for(uint32_t idx = 0; idx < pbufs_recvd; idx++)
        {
            p_holder = p;
            p = pbuf_dechain(p_holder);
            Emitter_EmitEvent(EVENT(UDPReceived));
        }
    }
    else
    {
        printf("\tConnection Closed\n");
    }

    Close();
}

extern void UDP_Retrieve( uint8_t * buffer, uint16_t len)
{
    assert(p_holder != NULL);
    memset(buffer, 0x00, len);
    pbuf_copy_partial(p_holder, buffer, len, 0);
    uint8_t num_dealloc = pbuf_free(p_holder);
    printf("\t%u chains deallocated\n", num_dealloc);
}

extern bool UDP_Send( uint8_t * buffer, uint16_t len, ip_addr_t ip, uint16_t port, critical_section_t * crit)
{
    bool ret = true;
    if( udp_pcb != NULL )
    {
        Close();
    }

    udp_pcb = udp_new();
    udp_recv( udp_pcb, Recv, NULL);
    struct pbuf * p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    memcpy(p->payload, buffer, len);

    cyw43_arch_lwip_begin();
    critical_section_enter_blocking(crit);
    err_t err = udp_sendto(udp_pcb, p,&ip,port);
    critical_section_exit(crit);
    cyw43_arch_lwip_end();
    
    pbuf_free(p);
    if(err != ERR_OK)
    {
        ret = false;
        Close();
    }

    return ret;
}


