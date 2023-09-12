#include "alarm.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "node_events.h"
#include "emitter.h"

static datetime_t dt;
static datetime_t alarm;

extern void Alarm_Init(void)
{
    printf("\tInitialising Alarm\n");
    rtc_init();
    memset(&dt, 0x00, sizeof(datetime_t));
}

extern void Alarm_SetClock(time_t * unixtime)
{
    printf("\tSetting Clock to %s\n", ctime(unixtime));
    struct tm * time = gmtime(unixtime);

    dt.year = time->tm_year + 1900U;
    dt.month = time->tm_mon;
    dt.day = time->tm_mon;
    dt.dotw = time->tm_wday;
    dt.hour = time->tm_hour;
    dt.min = time->tm_min;
    dt.sec = time->tm_sec;

    rtc_set_datetime(&dt);
}

extern void Alarm_Start(void)
{
}

extern void Alarm_Stop(void)
{
}

