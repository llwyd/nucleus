#include "assert.h"
#include "scratch.h"
#include <stdio.h>    
static uint8_t scratch_buffer[SCRATCH_BANKS][SCRATCH_SIZE] = {0};

extern void Scratch_Init(void)
{
    printf("Initialising Scratch...\n");
    printf("\t %u bytes\n", SCRATCH_SIZE);
    printf("\t %u banks\n", SCRATCH_BANKS);
    memset(scratch_buffer[0], 0x00, SCRATCH_SIZE);
    memset(scratch_buffer[1], 0x00, SCRATCH_SIZE);
}

extern uint8_t * const Scratch_Get(uint32_t bank)
{
    assert(bank < SCRATCH_BANKS);
    return scratch_buffer[bank];
}

