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
#include "idt.h"
#include "multiboot.h"
#include "lib.h"

static void debug_puts(const char*s){while(*s)outb(0xE9,(u8)*s++);}
static int has_word(const char*s,const char*w){if(!s||!w)return 0;usize n=kstrlen(w);for(;*s;s++)if(!kstrncmp(s,w,n)&&(!s[n]||s[n]==' '))return 1;return 0;}
static int boot_full(u32 magic,u32 mbi_addr){
    if(magic!=MULTIBOOT_BOOTLOADER_MAGIC)return 0;MultibootInfo*m=(MultibootInfo*)mbi_addr;
    if(!(m->flags&(1u<<2))||!m->cmdline)return 0;return has_word((const char*)m->cmdline,"full");
}
void kernel_main(u32 magic,u32 mbi_addr){
    AppId current=APP_TERMINAL;int mx=8,my=8;int full=boot_full(magic,mbi_addr);int heartbeat=0;int menu_focus=0;u32 clock_last=0;
    debug_puts("PCOS 0.7: kernel_main\n");
    idt_init();
    debug_puts("PCOS: video init\n");vga_init(magic,mbi_addr);
    debug_puts("PCOS: core apps/fs init\n");apps_init(magic,mbi_addr);
    debug_puts("PCOS: first UI draw\n");ui_draw(current);
    debug_puts("PCOS: UI ONLINE\n");
    timer_init(100);debug_puts("PCOS: timer online\n");
    keyboard_init();debug_puts("PCOS: keyboard online\n");
    if(full){
        debug_puts("PCOS: FULL drivers requested\n");
        ata_init();debug_puts("PCOS: ATA init done\n");
        mouse_init();debug_puts("PCOS: mouse init done\n");
        net_init();debug_puts("PCOS: network init done\n");
        sb16_init();debug_puts("PCOS: audio init done\n");
        ui_draw(current);if(mouse_available())vga_cursor_cell(mx,my);
    }else debug_puts("PCOS: SAFE mode - optional drivers deferred\n");
    debug_puts("PCOS: entering event loop\n");
    for(;;){
        int redraw=0;timer_poll();u32 now=timer_ticks();u32 hz=timer_hz();
        if(hz&&now-clock_last>=hz){clock_last=now;redraw=1;}
        if(!heartbeat&&now>=500){debug_puts("PCOS: ALIVE 5S\n");heartbeat=1;}
        net_poll();
        MouseEvent m=mouse_poll();
        if(m.moved||m.clicked){
            mx+=m.dx/3;my+=m.dy/5;if(mx<0)mx=0;if(mx>=VGA_W)mx=VGA_W-1;if(my<1)my=1;if(my>=VGA_H)my=VGA_H-1;
            if(m.clicked){
                if(mx<18&&my>=2&&my<2+APP_COUNT){current=(AppId)(my-2);menu_focus=0;}
                else{apps_mouse(current,mx,my,m.buttons,m.clicked);menu_focus=0;}
            }
            redraw=1;
        }
        KeyEvent e=keyboard_poll();
        if(e.pressed){
            if(e.special>=KEY_F1&&e.special<=KEY_F11){int a=e.special-KEY_F1;if(a<APP_COUNT){current=(AppId)a;menu_focus=0;}redraw=1;}
            else if(e.special==KEY_LAYOUT){redraw=1;}
            else if(e.special==KEY_TAB){menu_focus=!menu_focus;redraw=1;}
            else if(menu_focus){
                if(e.special==KEY_UP){current=(AppId)(((int)current+APP_COUNT-1)%APP_COUNT);redraw=1;}
                else if(e.special==KEY_DOWN){current=(AppId)(((int)current+1)%APP_COUNT);redraw=1;}
                else if(e.special==KEY_ENTER){menu_focus=0;redraw=1;}
                else{apps_key(current,e);redraw=1;}
            }else{apps_key(current,e);redraw=1;}
        }
        if(apps_tick(current))redraw=1;
        if(redraw){ui_draw(current);if(full&&mouse_available())vga_cursor_cell(mx,my);}cpu_pause();
    }
}
