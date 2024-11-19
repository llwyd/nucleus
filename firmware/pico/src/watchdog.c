#include "watchdog.h"

#define TIMEOUT_MS (6000U)

extern void Watchdog_Init(void)
{
#ifdef WATCHDOG_ENABLED
    printf("Watchdog Init\n");
    watchdog_enable(TIMEOUT_MS, 1);
#endif
}

extern void Watchdog_Kick(void)
{
#ifdef WATCHDOG_ENABLED
    watchdog_update();
#endif
}

