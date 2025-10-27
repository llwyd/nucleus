#ifndef UPTIME_H_
#define UPTIME_H_

#include "pico/stdlib.h"
#include "pico/binary_info.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

extern void Uptime_Init(void);
extern uint64_t Uptime_Refresh(void);
extern uint64_t Uptime_Get(void);
extern void Uptime_Encode(char * buffer, uint8_t buffer_len);

#endif /* UPTIME_H_ */

