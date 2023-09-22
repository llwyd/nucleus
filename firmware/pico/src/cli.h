#ifndef CLI_H_
#define CLI_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>
#include "pico/stdlib.h"
#include "state.h"

#define CLI_CMD_SIZE (64)

typedef struct
{
    const char * command;
    state_func_t state; 
}
cli_command_t;

extern void CLI_Init(uint8_t * buffer);
extern void CLI_Start(void);
extern void CLI_Stop(void);

#endif /* CLI_H_ */
