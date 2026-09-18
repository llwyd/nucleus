#ifndef SCRATCH_H
#define SCRATCH_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define SCRATCH_SIZE ( 1024U )

extern void Scratch_Init(void);
extern uint8_t * const Scratch_Get(void);

#endif /* SCRATCH_H */
