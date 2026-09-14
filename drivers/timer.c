#include "timer.h"
#include "io.h"
static u32 hzv=100,ticks=0;static u16 last=0;static int initd=0;
static u16 read_count(void){outb(0x43,0x00);u8 lo=inb(0x40),hi=inb(0x40);return (u16)(lo|((u16)hi<<8));}
void timer_init(u32 hz){if(hz<20)hz=20;if(hz>1000)hz=1000;hzv=hz;u32 div=1193182u/hz;if(div>65535)div=65535;outb(0x43,0x36);outb(0x40,(u8)div);outb(0x40,(u8)(div>>8));last=read_count();initd=1;}
void timer_poll(void){if(!initd)return;u16 now=read_count();if(now>last)ticks++;last=now;}
u32 timer_ticks(void){return ticks;}u32 timer_hz(void){return hzv;}
