#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"


int main()
{
    stdio_init_all();

    while( true )
    {
        printf("Hello, World!\n");
    }
    return 0;
}
