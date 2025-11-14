#include "tcp.h"
#include "node_events.h"
#include "emitter.h"
#include "eeprom.h"


#define BUFFER_SIZE     (128)
#define TCP_FIFO_LEN    (32U)

typedef struct
{
    struct pbuf * p;
}
tcp_pack_t;

typedef struct
{
    fifo_base_t base;
    tcp_pack_t queue[TCP_FIFO_LEN];
    tcp_pack_t in;
    tcp_pack_t out;
}
tcp_fifo_t;

static tcp_fifo_t tcp_fifo;
static critical_section_t * critical;

/* FIFO functions */
static void InitTCPFifo(tcp_fifo_t * fifo);
static void TCPEnqueue( fifo_base_t * const fifo );
static void TCPDequeue( fifo_base_t * const fifo );
static void TCPFlush( fifo_base_t * const fifo );
static void TCPPeek( fifo_base_t * const fifo );
static void FlushBuffer(tcp_t * tcp);

/* LWIP callback functions */
static err_t Sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t Recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void Error(void *arg, err_t err);
static err_t Connected(void *arg, struct tcp_pcb *tpcb, err_t err);

static err_t Sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    (void)arg;
    (void)tpcb;
    (void)len;
    printf("\tTCP: %u bytes ACK'd\n", len);
    Emitter_EmitEvent(EVENT(AckReceived));
    return ERR_OK;
}

static err_t Recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    (void)arg;
    (void)tpcb;
    (void)err;
    cyw43_arch_lwip_check();
    err_t ret = ERR_OK;
    printf("\terr: %d\n", err);
    critical_section_enter_blocking(critical);
    if( p != NULL )
    {
        printf("\tfifo fill: %lu\n",tcp_fifo.base.fill);
        uint16_t pbufs_recvd = pbuf_clen( p ); 
        printf("\t%u pbufs recvd\n", pbufs_recvd);
        for(uint32_t idx = 0; idx < pbufs_recvd; idx++)
        {
            assert(p->len < BUFFER_SIZE); 
            tcp_pack_t temp =
            {
                .p = p,
            };
            p = pbuf_dechain(temp.p);
            if(!FIFO_IsFull(&tcp_fifo.base))
            {
                FIFO_Enqueue(&tcp_fifo, temp);
                Emitter_EmitEvent(EVENT(TCPReceived));
            }
        }
    }
    else
    {
        printf("\tConnection Closed\n");
        Emitter_EmitEvent(EVENT(TCPDisconnected));
        ret = ERR_CLSD;
    }
    critical_section_exit(critical);

    return ret;
}

extern void TCP_Flush(tcp_t * tcp)
{
    FlushBuffer(tcp);
}

extern void TCP_Abort(tcp_t * tcp)
{
    printf("\tTCP: Aborting\n");
    cyw43_arch_lwip_begin();
    tcp_abort(tcp->pcb);
    tcp->pcb = NULL;
    cyw43_arch_lwip_end();
}
extern void TCP_Close(tcp_t * tcp)
{
    if(tcp->pcb != NULL)
    {
        printf("\tClosing TCP comms\n");
        cyw43_arch_lwip_begin();
        err_t close_err = tcp_close(tcp->pcb);
        tcp_abort(tcp->pcb);
        cyw43_arch_lwip_end();
        tcp->err = close_err;
        if( close_err != ERR_OK )
        {
            printf("\tFailed to close (%d)", close_err);
            TCP_Abort(tcp);
            tcp->pcb = NULL;
        }
        else
        {
            printf("\tClose Success!\n");
            tcp->pcb = NULL;
        }
    }
}

static void FlushBuffer(tcp_t * tcp)
{
    critical_section_enter_blocking(tcp->crit);
    printf("\tTCP: Flushing (%lu)\n", tcp_fifo.base.fill);
    const uint32_t fill = tcp_fifo.base.fill;
    for(uint32_t idx = 0; idx < fill; idx++)
    {
        tcp_pack_t t = FIFO_Dequeue(&tcp_fifo);
        tcp_recved(tcp->pcb, t.p->len);
        uint8_t num_dealloc = pbuf_free(t.p);
        printf("\t%u tcp chains dealloc\n", num_dealloc);
    }
    assert(FIFO_IsEmpty(&tcp_fifo.base));
    critical_section_exit(tcp->crit);
}

extern uint16_t TCP_Retrieve(tcp_t * tcp, uint8_t * buffer, uint16_t len)
{
    (void)tcp;
    critical_section_enter_blocking(tcp->crit);
    assert(!FIFO_IsEmpty(&tcp_fifo.base));
    memset(buffer, 0x00, len);
    tcp_pack_t t = FIFO_Dequeue(&tcp_fifo);

    uint16_t recv_len = t.p->len;
    printf("\t%u bytes received\n", recv_len);
    uint16_t copied = pbuf_copy_partial(t.p, buffer, t.p->len, 0);
    printf("\t%u bytes retrieved\n", copied);
    tcp_recved(tcp->pcb, t.p->len);
    uint8_t num_dealloc = pbuf_free(t.p);
    printf("\t%u tcp chains dealloc\n", num_dealloc);
    critical_section_exit(tcp->crit);
    return recv_len;
}

static void Error(void *arg, err_t err)
{
    (void)arg;
    (void)err;
    printf("\tTCP Error (%d)\n", (int16_t)err);
    switch(err)
    {
        case ERR_RST:
        {
            Emitter_EmitEvent(EVENT(TCPDisconnected));
            break;
        }
        default:
        {
            break;
        }
    }
}

static err_t Connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    (void)arg;
    (void)tpcb;
    printf("\tTCP Connected...");
    if( err == ERR_OK )
    {
        printf("OK\n");
        Emitter_EmitEvent(EVENT(TCPConnected));
    }
    else
    {
        printf("FAIL\n");
    }
    return err;
}

extern void TCP_Kick(tcp_t * tcp)
{
    cyw43_arch_lwip_begin();
    critical_section_enter_blocking(tcp->crit);
    err_t err = tcp_output(tcp->pcb);  
    critical_section_exit(tcp->crit);
    cyw43_arch_lwip_end();

    printf("\tTCP Kick: %d\n", err);
}

extern bool TCP_Send( tcp_t * tcp, uint8_t * buffer, uint16_t len )
{
    printf("\tAttempting to send %u bytes\n", len);
    bool success = true;

    if( buffer == NULL)
    {
        success = false;
        goto cleanup;
    }
    if(len == 0u)
    {
        success = false;
        goto cleanup;
    }
    cyw43_arch_lwip_begin();
    critical_section_enter_blocking(tcp->crit);
    
    uint16_t available = tcp_sndbuf(tcp->pcb);
    printf("\tTCP space available to output: %u\n", available);
    err_t err = tcp_write(tcp->pcb, buffer, len, TCP_WRITE_FLAG_COPY);
    tcp->err = err; 
    critical_section_exit(tcp->crit);
    cyw43_arch_lwip_end();
    if( err != ERR_OK )
    {
        printf("\tFailed to write (%d)\n", (int16_t)err);
        success = false;
        if(err == ERR_ARG)
        {
            printf("\tlwip pcb error\n");
            Emitter_EmitEvent(EVENT(PCBInvalid));

            /* This looks strange, but we dont want to kick off the retry timer
             * as we are going to nuke the connection anyway so return true
             */
            success = true;
        }
        else if(err == ERR_MEM)
        {
            printf("\tlwip memory error\n");
        }
        goto cleanup;
    }
    
    cyw43_arch_lwip_begin();
    critical_section_enter_blocking(tcp->crit);
    err = tcp_output(tcp->pcb);  
    tcp->err = err; 
    critical_section_exit(tcp->crit);
    cyw43_arch_lwip_end();
    if( err != ERR_OK )
    {
        printf("\tFailed to output\n");
        success = false;
        goto cleanup;
    }
    printf("\tSend successfully\n"); 
cleanup:
    return success;
}


extern bool TCP_Connect(tcp_t * tcp)
{
    bool ret = false;
    
    printf("\tAttempting Connection to %s port %d\n", ip4addr_ntoa(&tcp->ip), tcp->port);

    /* Initialise pcb struct */
    assert(tcp->pcb == NULL);
    tcp->pcb = tcp_new();
    if( tcp->pcb == NULL )
    {
        printf("\tFailed to create\n");
        ret = false;
        goto init_cleanup;
    }

    /* Define Callbacks */
    tcp_nagle_disable(tcp->pcb);
    tcp_sent(tcp->pcb, Sent);
    tcp_recv(tcp->pcb, Recv);
    tcp_err(tcp->pcb, Error);
    tcp->pcb->so_options |= SOF_KEEPALIVE;
    tcp->pcb->keep_intvl = 200;
    
    /* Attempt connection */
    cyw43_arch_lwip_begin();
    critical_section_enter_blocking(tcp->crit);
    err_t err = tcp_connect(tcp->pcb, &tcp->ip, tcp->port, Connected);
    critical_section_exit(tcp->crit);
    cyw43_arch_lwip_end();

    if(err==ERR_OK)
    {
        printf("\tTCP Initialising success, awaiting connection\n");
        ret = true;
    }
    else
    {
        printf("\tTCP Initialising failure, Retrying\n");
        Emitter_EmitEvent(EVENT(TCPDisconnected));
        ret = false;
    }

init_cleanup:
    return ret;
}

extern bool TCP_MemoryError(tcp_t * tcp)
{
    return (tcp->err == ERR_MEM);
}

extern void TCP_Init(tcp_t * tcp, char * buffer, uint16_t port, critical_section_t * crit)
{
    printf("Initialising TCP\n");
    printf("\tBroker IP: %s\n", buffer);
    tcp->pcb = NULL;
    ip4addr_aton(buffer, &tcp->ip);
    tcp->port = port;
    tcp->crit = crit;
    critical = crit;
    InitTCPFifo(&tcp_fifo);
}

static void InitTCPFifo(tcp_fifo_t * fifo)
{
    printf("Initialising MQTT FIFO\n");
    
    static const fifo_vfunc_t vfunc =
    {
        .enq = TCPEnqueue,
        .deq = TCPDequeue,
        .flush = TCPFlush,
        .peek = TCPPeek,
    };
    FIFO_Init( (fifo_base_t *)fifo, TCP_FIFO_LEN );
    
    fifo->base.vfunc = &vfunc;
    memset(fifo->queue, 0x00, TCP_FIFO_LEN * sizeof(fifo->in));
}

static void TCPEnqueue( fifo_base_t * const base )
{
    assert(base != NULL );
    ENQUEUE_BOILERPLATE( tcp_fifo_t, base );
}

static void TCPDequeue( fifo_base_t * const base )
{
    assert(base != NULL );
    DEQUEUE_BOILERPLATE( tcp_fifo_t, base );
}

static void TCPFlush( fifo_base_t * const base )
{
    assert(base != NULL );
    FLUSH_BOILERPLATE( tcp_fifo_t, base );
}

static void TCPPeek( fifo_base_t * const base )
{
    assert(base != NULL );
    PEEK_BOILERPLATE( tcp_fifo_t, base );
}

