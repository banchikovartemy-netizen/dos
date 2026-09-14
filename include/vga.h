#ifndef PCOS_VGA_H
#define PCOS_VGA_H
#include "types.h"
#define VGA_W 80
#define VGA_H 25

void vga_init(u32 magic,u32 mbi_addr);
void vga_set_theme(u8 theme);
u8 vga_theme(void);
void vga_clear(void);
void vga_put(int x,int y,char c,u8 attr);
void vga_text(int x,int y,const char *s,u8 attr);
void vga_text_clip(int x,int y,const char *s,int max,u8 attr);
void vga_fill(int x,int y,int w,int h,char c,u8 attr);
void vga_hline(int x,int y,int w,char c,u8 attr);
void vga_vline(int x,int y,int h,char c,u8 attr);
u8 vga_fg(void); u8 vga_dim(void); u8 vga_hi(void); u8 vga_bg(void);
int vga_framebuffer(void);
u32 vga_px_width(void); u32 vga_px_height(void); u32 vga_px_bpp(void);
void vga_cell_rect(int cx,int cy,int cw,int ch,int *x,int *y,int *w,int *h);
void vga_pixel(int x,int y,u32 rgb);
void vga_fill_px(int x,int y,int w,int h,u32 rgb);
u32 vga_rgb_fg(void); u32 vga_rgb_dim(void); u32 vga_rgb_hi(void); u32 vga_rgb_bg(void);
void vga_cursor_cell(int x,int y);
#endif
