#include "ntp.h"
#include "lwip/pbuf.h"
#include "lwip/dns.h"
#include "node_events.h"
#include "emitter.h"

#define NTP_IP ("pool.ntp.org")
#define NTP_PORT (123U)

static ip_addr_t ntp_addr;
static const char * url = NTP_IP;

static void DNSFound(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    ntp_addr = *ipaddr;
    Emitter_EmitEvent(EVENT(DNSReceived));
}

extern void NTP_Init( ntp_t * ntp )
{
    printf("Initialising NTP\n");
}

extern void NTP_RequestDNS( ntp_t * ntp )
{
    printf("\tRequesting IP for %s\n", url);
    dns_gethostbyname(url, &ntp_addr, DNSFound, NULL);
}

extern void NTP_PrintIP( ntp_t * ntp )
{
    printf("\tIP Found: %s\n",ipaddr_ntoa(&ntp_addr));
}

extern void NTP_Get( ntp_t * ntp )
{
}

extern void NTP_Decode( uint8_t * buffer )
{
}

