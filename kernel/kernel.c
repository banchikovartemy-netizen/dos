#include "types.h"
#include "apps.h"
#include "ui.h"
#include "keyboard.h"
#include "mouse.h"
#include "vga.h"
#include "net.h"
#include "sb16.h"
#include "timer.h"
#include "ata.h"
#include "io.h"
static void debug_puts(const char*s){while(*s)outb(0xE9,(u8)*s++);}
void kernel_main(u32 magic,u32 mbi_addr){AppId current=APP_TERMINAL;int mx=8,my=8;debug_puts("PCOS: kernel_main\n");vga_init(magic,mbi_addr);timer_init(100);mouse_init();ata_init();net_init();sb16_init();apps_init(magic,mbi_addr);debug_puts("PCOS: subsystems online\n");ui_draw(current);vga_cursor_cell(mx,my);for(;;){int redraw=0;timer_poll();net_poll();MouseEvent m=mouse_poll();if(m.moved||m.clicked){mx+=m.dx/3;my+=m.dy/5;if(mx<0)mx=0;if(mx>=VGA_W)mx=VGA_W-1;if(my<1)my=1;if(my>=VGA_H)my=VGA_H-1;if(m.clicked&&mx<17&&my>=4&&my<4+APP_COUNT)current=(AppId)(my-4);redraw=1;}KeyEvent e=keyboard_poll();if(e.pressed){if(e.special>=KEY_F1&&e.special<=KEY_F11)current=(AppId)(e.special-KEY_F1);else apps_key(current,e);redraw=1;}if(apps_tick(current))redraw=1;if(redraw){ui_draw(current);vga_cursor_cell(mx,my);}cpu_pause();}}
