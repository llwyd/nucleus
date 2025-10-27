#ifndef UDP_H_
#define UDP_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/udp.h"
#include "pico/critical_section.h"

extern void UDP_Recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, const ip_addr_t *addr, u16_t port);
extern bool UDP_Send( uint8_t * buffer, uint16_t len, ip_addr_t ip, uint16_t port, critical_section_t * crit);
extern void UDP_Retrieve( uint8_t * buffer, uint16_t len);

#endif /* UDP_H_ */

