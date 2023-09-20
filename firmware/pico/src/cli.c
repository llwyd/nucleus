#include "cli.h"
#include <string.h>
#include "emitter.h"
#include "node_events.h"

#define KEY_ENTER (0x0D)

uint8_t * command_buffer;
static uint8_t write_index;

static void rx_recv(void)
{
    while(uart_is_readable(uart0))
    {
        uint8_t latest_char = uart_getc(uart0);
        if( latest_char != KEY_ENTER )
        {
            assert(write_index < CLI_CMD_SIZE);

            /* -1 because the string needs to be null terminated*/
            if(write_index < (CLI_CMD_SIZE - 1U))
            {
                printf("%c", latest_char);
                command_buffer[write_index] = latest_char;
                write_index++;
            }
        }
        else
        {
            uart_puts(uart0, "\r\n");
            write_index = (CLI_CMD_SIZE - 1U);
            Emitter_EmitEvent(EVENT(HandleCommand));
        }
    }
}

extern void CLI_Start(void)
{
    memset(command_buffer, 0x00, CLI_CMD_SIZE);
    write_index = 0U;
    irq_set_enabled(UART0_IRQ, true);
    uart_puts(uart0, "\n$ ");
}

extern void CLI_Stop(void)
{
    irq_set_enabled(UART0_IRQ, false);
}

extern void CLI_Init(uint8_t * buffer)
{
    assert(buffer != NULL);
    printf("Initialising CLI Interface\n");
    command_buffer = buffer;
    irq_set_exclusive_handler(UART0_IRQ, rx_recv);
    uart_set_irq_enables(uart0, true, false);
}
