#ifndef DNS_H_
#define DNS_H_

#include "lwip/pbuf.h"
#include "lwip/dns.h"
#include "node_events.h"
#include "emitter.h"


extern ip_addr_t DNS_Get(void);
extern void DNS_Request( char * url );
extern void DNS_PrintIP( void );

#endif /* DNS_H_ */
