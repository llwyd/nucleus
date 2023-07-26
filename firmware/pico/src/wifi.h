#ifndef WIFI_H_
#define WIFI_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/critical_section.h"

extern bool WIFI_Init(void);

#endif /* WIFI_H_ */
