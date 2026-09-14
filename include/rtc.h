#ifndef PCOS_RTC_H
#define PCOS_RTC_H
#include "types.h"
typedef struct { u8 second,minute,hour,day,month; u16 year; } RtcTime;
void rtc_read(RtcTime *t);
void rtc_format_hud(char *out,u32 outn);
const char *rtc_day_phase(void);
#endif
