#include "rtl8139.h"
#include "pci.h"
#include "io.h"
#include "lib.h"
static u16 io=0;static int ok=0;static u8 macv[6];static u8 rxbuf[8192+16+1500] __attribute__((aligned(256)));static u8 txbuf[4][1536] __attribute__((aligned(16)));static u32 rxoff=0,txi=0;
int rtl8139_init(void){u8 b,s,f;if(!pci_find(0x10EC,0x8139,&b,&s,&f)){ok=0;return 0;}u32 bar=pci_read32(b,s,f,0x10);if(!(bar&1)){ok=0;return 0;}io=(u16)(bar&~3u);u32 cmd=pci_read32(b,s,f,0x04);cmd|=0x00000005u;pci_write32(b,s,f,0x04,cmd);outb(io+0x52,0);outb(io+0x37,0x10);for(u32 i=0;i<100000&&inb(io+0x37)&0x10;i++);for(int i=0;i<6;i++)macv[i]=inb(io+i);outl(io+0x30,(u32)rxbuf);outw(io+0x3C,0);outl(io+0x44,0x0000008Fu);outl(io+0x40,0x03000700u);outb(io+0x37,0x0C);rxoff=0;txi=0;ok=1;return 1;}
int rtl8139_ready(void){return ok;}void rtl8139_mac(u8 out[6]){for(int i=0;i<6;i++)out[i]=macv[i];}
int rtl8139_send(const u8*d,u32 n){if(!ok||!d||n>1514)return 0;u32 len=n<60?60:n;kmemset(txbuf[txi],0,len);kmemcpy(txbuf[txi],d,n);outl((u16)(io+0x20+txi*4),(u32)txbuf[txi]);outl((u16)(io+0x10+txi*4),len);txi=(txi+1)&3;return 1;}
int rtl8139_recv(u8*out,u32 max){if(!ok||!out)return 0;if(inb(io+0x37)&1)return 0;u8*p=rxbuf+rxoff;u16 status=(u16)(p[0]|((u16)p[1]<<8));u16 len=(u16)(p[2]|((u16)p[3]<<8));if(!(status&1)||len<4||len>1800){rxoff=0;outw(io+0x38,0xFFF0);return 0;}u32 data_len=(u32)len-4;if(data_len>max)data_len=max;kmemcpy(out,p+4,data_len);rxoff=(rxoff+(u32)len+4+3)&~3u;rxoff%=8192;outw(io+0x38,(u16)(rxoff-16));return (int)data_len;}
