#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "types.h"
#include "keyboard.h"
#include "fs.h"
#include "vga.h"
#include "timer.h"
#include "dos86.h"

const u8 *fs_read_path(const char*p,u32*n){(void)p;if(n)*n=0;return 0;}
int vga_framebuffer(void){return 0;}
void vga_cell_rect(int a,int b,int cc,int d,int*x,int*y,int*w,int*h){*x=a;*y=b;*w=cc;*h=d;}
void vga_pixel(int x,int y,u32 q){(void)x;(void)y;(void)q;}
void vga_put(int x,int y,char q,u8 a){(void)x;(void)y;(void)q;(void)a;}
u8 vga_fg(void){return 7;} u8 vga_bg(void){return 0;} u8 vga_hi(void){return 15;} u8 vga_dim(void){return 8;}
u32 timer_ticks(void){static u32 x;return x++;} u32 timer_hz(void){return 100;}
int sb16_play_u8(const u8*s,u32 n,u32 r){(void)s;(void)n;(void)r;return 1;}

static u8 *read_all(const char*p,u32*n){FILE*f=fopen(p,"rb");long z;u8*b;if(!f)return 0;fseek(f,0,SEEK_END);z=ftell(f);rewind(f);b=(u8*)malloc((size_t)z);if(!b){fclose(f);return 0;}fread(b,1,(size_t)z,f);fclose(f);*n=(u32)z;return b;}
int main(void){char out[1024];u32 n;u8*b;
 b=read_all("assets/pc/games/hello.com",&n);if(!b||!dos86_run(b,n,"hello.com",out,sizeof(out))||!strstr(out,"RUNTIME ONLINE"))return 2;free(b);
 b=read_all("assets/pc/games/hello.exe",&n);if(!b||!dos86_run(b,n,"hello.exe",out,sizeof(out))||!strstr(out,"MZ EXE LOADER ONLINE"))return 3;free(b);
 b=read_all("assets/pc/games/vga13.com",&n);if(!b||!dos86_start(b,n,"vga13.com"))return 4;for(int i=0;i<20;i++)dos86_step(10000);if(!dos86_active())return 5;{KeyEvent e={.ch='x',.pressed=1};dos86_key(e);}for(int i=0;i<20&&dos86_active();i++)dos86_step(10000);free(b);if(dos86_active())return 6;
 puts("PASS: DOS86 COM, MZ EXE, VGA13 and keyboard wait/resume");return 0;}
