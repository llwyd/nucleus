#include "comms.h"

#define MQTT_PORT ( 1883 )

#define BUFFER_SIZE (256)
#define POLL_PERIOD (2)

static uint8_t send_buffer[ BUFFER_SIZE ];
static uint8_t recv_buffer[ BUFFER_SIZE ];

static ip_addr_t remote_addr;
static struct tcp_pcb * tcp_pcb;

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
    return ERR_OK;
}

static void Error(void *arg, err_t err)
{
    printf("\tTCP Error\n");
}

static err_t Connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
    printf("\tTCP Connected\n");
    return ERR_OK;
}

static err_t Poll(void *arg, struct tcp_pcb *tpcb)
{
    printf("\tTCP Poll\n");

    return ERR_OK;
}

extern void Comms_Init(void)
{
    /* Init */
    ip4addr_aton(MQTT_BROKER_IP, &remote_addr);
    
    printf("Initialising TCP Comms\n");
    printf("Attempting Connection to %s port %d\n", ip4addr_ntoa(&remote_addr), MQTT_PORT);

    /* Initialise pcb struct */
    tcp_pcb = tcp_new_ip_type(IP_GET_TYPE(&remote_addr));
    if( tcp_pcb == NULL )
    {
        printf("Failed to create\n");
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

    assert(err==ERR_OK);
}
