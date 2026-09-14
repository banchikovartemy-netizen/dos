#include "mouse.h"
#include "io.h"
static int present=0;static u8 packet[3];static int pi=0;static u8 last_buttons=0;
static int wait_in(void){for(u32 i=0;i<100000;i++)if(!(inb(0x64)&2))return 1;return 0;}static int wait_out(void){for(u32 i=0;i<100000;i++)if(inb(0x64)&1)return 1;return 0;}static void cmd(u8 v){if(wait_in())outb(0x64,v);}static void dat(u8 v){if(wait_in())outb(0x60,v);}static u8 rd(void){return wait_out()?inb(0x60):0;}static int send_mouse(u8 v){cmd(0xD4);dat(v);return rd()==0xFA;}
void mouse_init(void){cmd(0xA8);cmd(0x20);u8 c=rd();c|=2;c&=(u8)~0x20;cmd(0x60);dat(c);if(!send_mouse(0xF6))return;if(!send_mouse(0xF4))return;present=1;}int mouse_available(void){return present;}
MouseEvent mouse_poll(void){MouseEvent e={0,0,last_buttons,0,0};if(!present)return e;u8 st=inb(0x64);if(!(st&1)||!(st&0x20))return e;u8 b=inb(0x60);if(pi==0&&!(b&8))return e;packet[pi++]=b;if(pi<3)return e;pi=0;if(packet[0]&0xC0)return e;e.dx=(i8)packet[1];e.dy=(i8)(-(i8)packet[2]);e.buttons=packet[0]&7;e.moved=(e.dx||e.dy)?1:0;e.clicked=(u8)((e.buttons&1)&&!(last_buttons&1));last_buttons=e.buttons;return e;}
