#ifndef ENVIRONMENT_H_
#define ENVIRONMENT_H_


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"

extern void Enviro_Init(void);
extern void Enviro_Read(void);
extern void Enviro_Print(void);

#endif /* ENVIRONMENT_H_ */
