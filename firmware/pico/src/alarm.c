#include "alarm.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/rtc.h"
#include "pico/stdlib.h"
#include "pico/util/datetime.h"
#include "node_events.h"
#include "emitter.h"

#define NUM_MINUTES (12U)

static datetime_t dt;
static datetime_t alarm;

uint8_t pos = 0U;
int8_t minutes[NUM_MINUTES] = {0,5,10,15,20,25,30,35,40,45,50,55};

static void Callback(void)
{
    Emitter_EmitEvent(EVENT(AlarmElapsed));
    pos++;
    if( pos >= NUM_MINUTES )
    {
        pos = 0;
    }
    alarm.min = minutes[pos];
    rtc_set_alarm(&alarm, Callback);
}



extern void Alarm_Init(void)
{
    printf("\tInitialising Alarm\n");
    rtc_init();
    memset(&dt, 0x00, sizeof(datetime_t));
    
}

extern void Alarm_SetClock(time_t * unixtime)
{
    struct tm * time = gmtime(unixtime);

    dt.year = time->tm_year + 1900U;
    dt.month = time->tm_mon + 1U;
    dt.day = time->tm_mday;
    dt.dotw = time->tm_wday;
    dt.hour = time->tm_hour;
    dt.min = time->tm_min;
    dt.sec = time->tm_sec;

    rtc_set_datetime(&dt);

    alarm.year = -1;
    alarm.month = -1;
    alarm.day = -1;
    alarm.dotw = -1;
    alarm.hour = -1;
    alarm.min = 0;
    alarm.sec = 0;
}

extern void Alarm_Start(void)
{
    printf("\tStarting Alarm\n");
    rtc_get_datetime(&dt);
    
    pos = (dt.min / 5) + 1;
    if( pos >= NUM_MINUTES )
    {
        pos = 0;
    }

    alarm.min = minutes[pos];
    rtc_set_alarm(&alarm, Callback);
}

extern void Alarm_PrintCurrent(uint8_t * buffer, uint16_t len)
{
    memset(buffer, 0x00, len);
    rtc_get_datetime(&dt);
    datetime_to_str((char*)buffer, len, &dt);
    printf("%s\n", buffer);
}

extern void Alarm_FormatTime(uint8_t * buffer, uint16_t len)
{
    memset(buffer, 0x00, len);
    rtc_get_datetime(&dt);
    snprintf((char*)buffer, len, "%02u:%02u:%02u",dt.hour,dt.min,dt.sec);
}

extern void Alarm_FormatDate(uint8_t * buffer, uint16_t len)
{
    memset(buffer, 0x00, len);
    rtc_get_datetime(&dt);
    snprintf((char*)buffer, len, "%02u-%02u-%02u",dt.year,dt.month,dt.day);
}

extern time_t Alarm_GetUnixTime(void)
{
    rtc_get_datetime(&dt);
    struct tm time =
    {
        .tm_year = dt.year - 1900U,
        .tm_mon = dt.month - 1U,
        .tm_mday = dt.day,
        .tm_wday = dt.dotw,
        .tm_hour = dt.hour,
        .tm_min = dt.min,
        .tm_sec = dt.sec,    
    };
    time_t utime = mktime(&time);

    return utime;
}

extern uint64_t Alarm_EncodeUnixTime(char * buffer, uint8_t buffer_len)
{
    assert(buffer != NULL);
    uint64_t utime = (uint64_t)Alarm_GetUnixTime();

    memset(buffer,0x00, buffer_len);
    
    snprintf(buffer, buffer_len,"{\"ts\":%llu}",
            utime);

    return utime;
}

extern void Alarm_Stop(void)
{
    printf("\tStopping Alarm\n");
    rtc_disable_alarm();
}

