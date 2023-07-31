#include "comms.h"

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
    printf("\tReceived %d bytes\n", p->tot_len);

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
    uint16_t full_packet_size = 0;
    /* Message Template for mqtt connect */
    uint8_t mqtt_template_connect[] =
    {
                0x10,                                   /* connect */
                0x00,                                   /* payload size */
                0x00, 0x06,                             /* Protocol name size */
                0x4d, 0x51, 0x49, 0x73, 0x64, 0x70,     /* MQIsdp */
                0x03,                                   /* Version MQTT v3.1 */
                0x02,                                   /* Fire and forget */
                0x00, MQTT_TIMEOUT,                     /* Keep alive timeout */
    };
    uint8_t * msg_ptr = send_buffer;
    uint16_t packet_size = (uint16_t)sizeof( mqtt_template_connect );
    uint16_t name_size = strlen( client_name );

    memcpy( msg_ptr, mqtt_template_connect, packet_size );
    msg_ptr+= packet_size;
    msg_ptr++;
    *msg_ptr++ = (uint8_t)(name_size & 0xFF);
    memcpy(msg_ptr, client_name, name_size);
                
    uint16_t total_packet_size = packet_size + name_size;
    send_buffer[1] = (uint8_t)(total_packet_size&0xFF);
    full_packet_size = total_packet_size + 2;

    err_t err = tcp_write(tcp_pcb, send_buffer, full_packet_size, TCP_WRITE_FLAG_COPY);

    if( err != ERR_OK )
    {
        printf("\nFailed to write\n");
    }
    else
    {
        for( uint32_t idx = 0; idx < full_packet_size; idx++)
        {
            printf("0x%x", send_buffer[idx]);
        }
    }
}

extern bool Comms_Send( uint8_t * buffer, uint16_t len )
{
    return true;
}

extern bool Comms_Recv( uint8_t * buffer, uint16_t len )
{
    return true;
}

extern bool Comms_Init(void)
{
    bool ret = false;
    connected = false;
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

