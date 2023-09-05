#ifndef NTP_H_
#define NTP_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include <time.h>

typedef struct
{
    bool (*send)( uint8_t * buffer, uint16_t len, char * ip, uint16_t port);
}
ntp_t;

extern void NTP_Init( ntp_t * ntp );
extern void NTP_Get( ntp_t * ntp );
extern void NTP_PrintIP( ntp_t * ntp );
extern void NTP_Decode( uint8_t * buffer );
extern void NTP_RequestDNS( ntp_t * ntp );

#endif /* NTP_H_ */

