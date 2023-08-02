#include "comms.h"
#include "node_events.h"
#include "emitter.h"

#define MQTT_PORT ( 1883 )
#define MQTT_TIMEOUT ( 0xb4 )

#define BUFFER_SIZE (256)
#define POLL_PERIOD (2)

static uint8_t send_buffer[ BUFFER_SIZE ];
static uint8_t recv_buffer[ BUFFER_SIZE ];

static ip_addr_t remote_addr;
static struct tcp_pcb * tcp_pcb;
static bool connected = false;
static char * client_name = "pico";
static msg_fifo_t * msg_fifo;

/* LWIP callback functions */
static err_t Sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static err_t Recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void Error(void *arg, err_t err);
static err_t Connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static err_t Poll(void *arg, struct tcp_pcb *tpcb);

static err_t Sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
    printf("\tTCP Sent\n");
    return ERR_OK;
}

static err_t Recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    printf("\tTCP Recv\n");
    printf("\tReceived %d bytes total\n", p->tot_len);

    for(; p != NULL; p = p->next )
    {
        printf("\tCopying message len %d bytes\n\t", p->len);
        for( uint32_t idx = 0; idx < p->len; idx++)
        {
            printf("0x%x,", ((uint8_t*)p->payload)[idx]);
        }
        printf("\b \n");
    }
    
    Emitter_EmitEvent(EVENT(MessageReceived));
    return ERR_OK;
}

static void Error(void *arg, err_t err)
{
    printf("\tTCP Error\n");
    connected = false;
}

static err_t Connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    printf("\tTCP Connected...");
    connected = true;
    if( err == ERR_OK )
    {
        printf("OK\n");
    }
    else
    {
        printf("FAIL\n");
    }
    return ERR_OK;
}

static err_t Poll(void *arg, struct tcp_pcb *tpcb)
{
    printf("\tTCP Poll\n");

    return ERR_OK;
}

extern void Comms_MQTTConnect(void)
{
}

extern bool Comms_Send( uint8_t * buffer, uint16_t len )
{
    err_t err = tcp_write(tcp_pcb, buffer, len, TCP_WRITE_FLAG_COPY);
    bool success = true;
    if( err != ERR_OK )
    {
        printf("\nFailed to write\n");
        success = false;
    }
    return success;
}

extern bool Comms_Recv( uint8_t * buffer, uint16_t len )
{
    return true;
}

extern bool Comms_TCPInit(void)
{
    bool ret = false;
    connected = false;
    /* Init */
    ip4addr_aton(MQTT_BROKER_IP, &remote_addr);
    
    printf("\tInitialising TCP Comms\n");
    printf("\tAttempting Connection to %s port %d\n", ip4addr_ntoa(&remote_addr), MQTT_PORT);

    /* Initialise pcb struct */
    tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&remote_addr));
    if( tcp_pcb == NULL )
    {
        printf("\tFailed to create\n");
        assert(false);
    }

    /* Define Callbacks */
    // tcp_arg
    tcp_sent(tcp_pcb, Sent);
    tcp_poll(tcp_pcb, Poll, POLL_PERIOD);
    tcp_recv(tcp_pcb, Recv);
    tcp_err(tcp_pcb, Error);

    /* Attempt connection */
    cyw43_arch_lwip_begin();
    err_t err = tcp_connect(tcp_pcb, &remote_addr, MQTT_PORT, Connected);
    cyw43_arch_lwip_end();

    if(err==ERR_OK)
    {
        printf("\tTCP Initialising success, awaiting connection\n");
        ret = true;
    }
    else
    {
        printf("\tTCP Initialising failure, Retrying\n");
        err_t close_err = tcp_close(tcp_pcb);
        if( close_err != ERR_OK )
        {
            tcp_abort(tcp_pcb);
        }
        tcp_pcb = NULL;
        ret = false;
    }

    return ret;
}

extern bool Comms_CheckStatus(void)
{
    return connected;
}

extern void Comms_Init(msg_fifo_t * fifo)
{
    printf("Initialising Comms\n");
    msg_fifo = fifo;
}

