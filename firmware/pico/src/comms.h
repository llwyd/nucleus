#ifndef COMMS_H_
#define COMMS_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "secret_keys.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"

extern bool Comms_Init(void);

#endif /* COMMS_H_ */
