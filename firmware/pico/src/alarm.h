#ifndef ALARM_H_
#define ALARM_H_

#include <time.h>
extern void Alarm_Init(void);
extern void Alarm_SetClock(time_t * unixtime);
extern void Alarm_Start(void);
extern void Alarm_Stop(void);
extern void Alarm_PrintCurrent(uint8_t * buffer, uint16_t len);
extern void Alarm_FormatTime(uint8_t * buffer, uint16_t len);
extern void Alarm_FormatDate(uint8_t * buffer, uint16_t len);
extern void Alarm_CalculateDST(void);
extern time_t Alarm_GetUnixTime(void);
extern void Alarm_EncodeUnixTime(char * buffer, uint8_t buffer_len);

#endif /* ALARM_H_ */
