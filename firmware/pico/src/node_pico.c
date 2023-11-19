#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <assert.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/critical_section.h"
#include "hardware/gpio.h"

#include "meta.h"
#include "daemon.h"
#include "gpio.h"
#include "config.h"

static void PrintStartupInfo(void)
{
    printf("Pico sensor node\n");
    printf("Git hash: %s\n", META_GITHASH);
    printf("Build time: %s %s\n", META_DATESTAMP, META_TIMESTAMP);
}

int main()
{
    stdio_init_all();
    PrintStartupInfo();
    GPIO_Init();
    
    if(gpio_get(CONFIG_PIN))
    { 
        Daemon_Run();
    }
    else
    {
        Config_Run();
    }

    assert(false);
    return 0;
}
