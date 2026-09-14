#ifndef PCOS_TIMER_H
#define PCOS_TIMER_H
#include "types.h"
void timer_init(u32 hz);
void timer_poll(void);
u32 timer_ticks(void);
u32 timer_hz(void);
#endif
