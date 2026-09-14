#include "pci.h"
#include "io.h"
u32 pci_read32(u8 bus,u8 slot,u8 func,u8 off){u32 a=0x80000000u|((u32)bus<<16)|((u32)slot<<11)|((u32)func<<8)|(off&0xFC);outl(0xCF8,a);return inl(0xCFC);}
void pci_write32(u8 bus,u8 slot,u8 func,u8 off,u32 value){u32 a=0x80000000u|((u32)bus<<16)|((u32)slot<<11)|((u32)func<<8)|(off&0xFC);outl(0xCF8,a);outl(0xCFC,value);}
int pci_find(u16 vendor,u16 device,u8 *bus,u8 *slot,u8 *func){for(u16 b=0;b<256;b++)for(u8 s=0;s<32;s++)for(u8 f=0;f<8;f++){u32 v=pci_read32((u8)b,s,f,0);if((u16)v==0xFFFF){if(f==0)break;continue;}if((u16)v==vendor&&(u16)(v>>16)==device){*bus=(u8)b;*slot=s;*func=f;return 1;}if(f==0&&!(pci_read32((u8)b,s,f,0x0C)&0x00800000u))break;}return 0;}
