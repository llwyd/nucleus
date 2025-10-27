#ifndef NTP_H_
#define NTP_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/udp.h"
#include <time.h>

#define NTP_IP ("pool.ntp.org")
#define NTP_PORT (123U)

typedef struct
{
    uint32_t id;
    time_t reference;
    time_t origin;
    time_t receive;
    time_t transmit;
} 
ntp_t;

extern void NTP_Encode( uint8_t * buffer );
extern void NTP_Decode( uint8_t * buffer, ntp_t * ntp );
extern void NTP_Print( ntp_t * ntp );

#endif /* NTP_H_ */

