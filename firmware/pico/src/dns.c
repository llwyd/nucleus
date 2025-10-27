#include "dns.h"

static ip_addr_t dns_addr;

static void DNSFound(const char *name, const ip_addr_t *ipaddr, void *callback_arg)
{
    (void)name;
    (void)callback_arg;
    dns_addr = *ipaddr;
    Emitter_EmitEvent(EVENT(DNSReceived));
}

extern ip_addr_t DNS_Get(void)
{
    return dns_addr;
}

extern void DNS_Request( char * url )
{
    printf("\tRequesting IP for %s\n", url);
    memset(&dns_addr, 0x00, sizeof(ip_addr_t));
    dns_gethostbyname(url, &dns_addr, DNSFound, NULL);
}

extern void DNS_PrintIP( void )
{
    printf("\tIP Found: %s\n",ipaddr_ntoa(&dns_addr));
}

