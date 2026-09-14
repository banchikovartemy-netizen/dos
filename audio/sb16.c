#include "sb16.h"
#include "io.h"
#include "lib.h"
#define SB 0x220
static int ready=0;static u8 dma_buf[65536] __attribute__((aligned(65536)));
static void delay(void){for(volatile int i=0;i<20000;i++)io_wait();}
static int dsp_write(u8 v){for(u32 i=0;i<100000;i++)if(!(inb(SB+0x0C)&0x80)){outb(SB+0x0C,v);return 1;}return 0;}
int sb16_init(void){outb(SB+0x06,1);delay();outb(SB+0x06,0);for(u32 i=0;i<100000;i++)if(inb(SB+0x0E)&0x80){if(inb(SB+0x0A)==0xAA){ready=1;dsp_write(0xD1);return 1;}break;}ready=0;return 0;}
int sb16_ready(void){return ready;}
static void dma1(const u8*buf,u16 count){u32 a=(u32)buf;outb(0x0A,0x05);outb(0x0C,0);outb(0x0B,0x49);outb(0x02,(u8)a);outb(0x02,(u8)(a>>8));outb(0x83,(u8)(a>>16));u16 c=(u16)(count-1);outb(0x03,(u8)c);outb(0x03,(u8)(c>>8));outb(0x0A,0x01);}
int sb16_play_u8(const u8*s,u32 count,u32 rate){if(!ready||!s||!count)return 0;if(count>65000)count=65000;kmemcpy(dma_buf,s,count);dma1(dma_buf,(u16)count);if(rate<4000)rate=4000;if(rate>22050)rate=22050;u8 tc=(u8)(256u-(1000000u/rate));dsp_write(0x40);dsp_write(tc);dsp_write(0x14);u16 c=(u16)(count-1);dsp_write((u8)c);dsp_write((u8)(c>>8));return 1;}
