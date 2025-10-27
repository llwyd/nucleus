#include "uptime.h"
#include <string.h>
#include "meta.h"

static absolute_time_t timestamp;
static uint32_t last_uptime_ms;
static uint64_t uptime_ms;

extern void Uptime_Init(void)
{
    printf("Initialising Uptime tracker\n");
    last_uptime_ms = 0U;
    uptime_ms = 0UL;
    timestamp = get_absolute_time();
}

extern uint64_t Uptime_Get(void)
{
    return uptime_ms;
}

extern void Uptime_Encode(char * buffer, uint8_t buffer_len)
{
    assert(buffer != NULL);
    memset(buffer,0x00, buffer_len);
    snprintf(buffer, buffer_len,"%llu", uptime_ms);
    
    snprintf(buffer, buffer_len,
            "{\"ut\":%llu,\"gt\":\"%s\"}",
            uptime_ms,
            META_GITHASH);
}

extern uint64_t Uptime_Refresh(void)
{
    timestamp = get_absolute_time();

    const uint32_t latest_ms = to_ms_since_boot(timestamp);
    const uint32_t uptime_delta_ms = latest_ms - last_uptime_ms;

    uptime_ms += (uint64_t)uptime_delta_ms;
    last_uptime_ms = latest_ms;

    return uptime_ms;
}

