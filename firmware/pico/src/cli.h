#ifndef CLI_H_
#define CLI_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "pico/stdlib.h"

#define CLI_CMD_SIZE (64)

extern void CLI_Init(uint8_t * buffer);
extern void CLI_Start(void);
extern void CLI_Stop(void);

#endif /* CLI_H_ */
