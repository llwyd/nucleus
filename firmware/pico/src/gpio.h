#ifndef GPIO_H_
#define GPIO_H_

#include "node_events.h"
#include "emitter.h"
#include "hardware/gpio.h"
#include <stdint.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "pico/binary_info.h"

extern void GPIO_Init(void);

#endif /* GPIO_H_ */
