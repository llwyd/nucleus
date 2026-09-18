#include "assert.h"
#include "scratch.h"
    
static uint8_t scratch_buffer[SCRATCH_SIZE] = {0};

extern void Scratch_Init(void)
{
    memset(scratch_buffer, 0x00, SCRATCH_SIZE);
}

extern uint8_t * const Scratch_Get(void)
{
    return scratch_buffer;
}
