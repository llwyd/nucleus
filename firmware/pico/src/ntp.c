#include "ntp.h"

#define NTP_IP ("pool.ntp.org")
#define NTP_PORT (123U)

extern void NTP_Init( ntp_t * ntp )
{
    printf("Initialising NTP\n");
}

extern void NTP_Get( ntp_t * ntp )
{
}

extern void NTP_Decode( uint8_t * buffer )
{
}

