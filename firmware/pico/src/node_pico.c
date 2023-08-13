#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/critical_section.h"

#include "daemon.h"

int main()
{
    Daemon_Run();
    return 0;
}
