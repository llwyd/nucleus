#include "ntp.h"
#include <time.h>

#define NTP2UNIX (((70U * 365U) + 17U) * 86400U)

extern void NTP_Encode( uint8_t * buffer )
{
    memset(buffer, 0x00, 48U);
    buffer[0] = 0x23;
}

extern void NTP_Decode( uint8_t * buffer, ntp_t * ntp )
{
    uint32_t * ptr = (uint32_t *)buffer;
    uint32_t raw[12U];
    for(uint32_t idx = 0; idx < 12U; idx++)
    {
        raw[idx] = __builtin_bswap32(ptr[idx]);
    }

    ntp->reference  = (time_t)(raw[4] - NTP2UNIX);
    ntp->origin     = (time_t)(raw[6] - NTP2UNIX);
    ntp->receive    = (time_t)(raw[8] - NTP2UNIX);
    ntp->transmit   = (time_t)(raw[10] - NTP2UNIX);
}

extern void NTP_Print(ntp_t * ntp)
{
    printf("%s", ctime(&ntp->reference));
    printf("%s", ctime(&ntp->origin));
    printf("%s", ctime(&ntp->receive));
    printf("%s", ctime(&ntp->transmit));
}

