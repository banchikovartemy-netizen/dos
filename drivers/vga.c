#include "vga.h"
#include "multiboot.h"
#include "lib.h"
#include "io.h"

static volatile u16 *const VGA=(u16*)0xB8000;
static u8 theme_id=0;
static int fb_on=0;
static u8 *fb=0;
static u32 fb_w=0,fb_h=0,fb_pitch=0,fb_bpp=0;
static u8 rpos=16,rsize=8,gpos=8,gsize=8,bpos=0,bsize=8;
static int scale=1, off_x=0, off_y=0;

static const u8 themes[][4]={{4,8,12,0},{2,8,10,0},{6,8,14,0},{3,8,11,0},{7,8,15,0}};
static const u32 rgb_themes[][4]={
 {0xB01010,0x501010,0xFF3B2F,0x000000},
 {0x20B060,0x104828,0x5BFF91,0x000000},
 {0xC08018,0x58400C,0xFFD060,0x000000},
 {0x2090B8,0x183C48,0x7BE7FF,0x000000},
 {0xB8B8B8,0x484848,0xFFFFFF,0x000000}
};

u8 vga_fg(void){return themes[theme_id][0];} u8 vga_dim(void){return themes[theme_id][1];}
u8 vga_hi(void){return themes[theme_id][2];} u8 vga_bg(void){return themes[theme_id][3];}
u8 vga_theme(void){return theme_id;}
u32 vga_rgb_fg(void){return rgb_themes[theme_id][0];} u32 vga_rgb_dim(void){return rgb_themes[theme_id][1];}
u32 vga_rgb_hi(void){return rgb_themes[theme_id][2];} u32 vga_rgb_bg(void){return rgb_themes[theme_id][3];}

void vga_set_theme(u8 t){theme_id=t%(sizeof(themes)/sizeof(themes[0]));}
static void disable_hw_cursor(void){outb(0x3D4,0x0A);outb(0x3D5,(u8)(inb(0x3D5)|0x20));}

static u8 glyph_row(char c,int row){
    if(c>='a'&&c<='z')c=(char)(c-'a'+'A');
    static const u8 az[26][7]={
      {14,17,17,31,17,17,17},{30,17,17,30,17,17,30},{14,17,16,16,16,17,14},{30,17,17,17,17,17,30},
      {31,16,16,30,16,16,31},{31,16,16,30,16,16,16},{14,17,16,23,17,17,15},{17,17,17,31,17,17,17},
      {31,4,4,4,4,4,31},{7,2,2,2,18,18,12},{17,18,20,24,20,18,17},{16,16,16,16,16,16,31},
      {17,27,21,21,17,17,17},{17,25,21,19,17,17,17},{14,17,17,17,17,17,14},{30,17,17,30,16,16,16},
      {14,17,17,17,21,18,13},{30,17,17,30,20,18,17},{15,16,16,14,1,1,30},{31,4,4,4,4,4,4},
      {17,17,17,17,17,17,14},{17,17,17,17,17,10,4},{17,17,17,21,21,21,10},{17,17,10,4,10,17,17},
      {17,17,10,4,4,4,4},{31,1,2,4,8,16,31}
    };
    static const u8 dg[10][7]={
      {14,17,19,21,25,17,14},{4,12,4,4,4,4,14},{14,17,1,2,4,8,31},{30,1,1,14,1,1,30},{2,6,10,18,31,2,2},
      {31,16,16,30,1,1,30},{14,16,16,30,17,17,14},{31,1,2,4,8,8,8},{14,17,17,14,17,17,14},{14,17,17,15,1,1,14}
    };
    if(row<0||row>6)return 0;
    if(c>='A'&&c<='Z')return az[c-'A'][row];
    if(c>='0'&&c<='9')return dg[c-'0'][row];
    switch(c){
      case ' ':return 0; case '-':return row==3?31:0; case '_':return row==6?31:0; case '=':return (row==2||row==4)?31:0;
      case '|':return 4; case '+':return (row==3)?31:4; case '.':return row==6?4:0; case ',':return row==6?4:(row==5?4:0);
      case ':':return (row==2||row==5)?4:0; case ';':return (row==2||row==5||row==6)?4:0; case '/':return (u8)(1u<<(4-(row*4/6)));
      case '\\':return (u8)(1u<<(row*4/6)); case '[':return (row==0||row==6)?14:8; case ']':return (row==0||row==6)?14:2;
      case '(':return (row==0||row==6)?2:(row==1||row==5?4:8); case ')':return (row==0||row==6)?8:(row==1||row==5?4:2);
      case '<':return row==3?8:(row==2||row==4?4:(row==1||row==5?2:0)); case '>':return row==3?2:(row==2||row==4?4:(row==1||row==5?8:0));
      case '!':return row<5?4:(row==6?4:0); case '?':return row==0?14:(row==1?17:(row==2?2:(row==3?4:(row==4?4:(row==6?4:0)))));
      case '#':return (row==2||row==4)?31:10; case '*':return row==3?31:(row==2||row==4?10:0); case '@':return row==0||row==6?14:(row==1?17:(row>=2&&row<=4?23:16));
      case '$':return row==0||row==6?4:(row==1?15:(row==2?20:(row==3?14:(row==4?5:30))));
      case '%':return row==0?17:(row==1?2:(row==2?4:(row==3?4:(row==4?8:17)))); case '^':return row==0?4:(row==1?10:17);
      case '&':return row==0?12:(row==1?18:(row==2?20:(row==3?8:(row==4?21:(row==5?18:13))))); case '~':return row==3?9:(row==2?22:0);
      case '\'':return row<2?4:0; case '"':return row<2?10:0; case '`':return row==0?8:(row==1?4:0);
      default:return (row==0||row==6)?31:(row==1||row==5?17:0);
    }
}

static u32 comp(u32 c,u8 pos,u8 bits){ if(bits>=8)return ((c&255u)<<pos); u32 max=(1u<<bits)-1u; return (((c&255u)*max/255u)<<pos); }
static u32 pack_rgb(u32 rgb){u32 r=(rgb>>16)&255,g=(rgb>>8)&255,b=rgb&255;return comp(r,rpos,rsize)|comp(g,gpos,gsize)|comp(b,bpos,bsize);}

void vga_init(u32 magic,u32 mbi_addr){
    disable_hw_cursor();
    if(magic!=MULTIBOOT_BOOTLOADER_MAGIC)return;
    MultibootInfo *m=(MultibootInfo*)mbi_addr;
    if((m->flags&MBI_FLAG_FB)&&m->framebuffer_addr&&m->framebuffer_width&&m->framebuffer_height&&m->framebuffer_type==1){
        fb_on=1; fb=(u8*)(u32)m->framebuffer_addr; fb_w=m->framebuffer_width; fb_h=m->framebuffer_height; fb_pitch=m->framebuffer_pitch; fb_bpp=m->framebuffer_bpp;
        rpos=m->framebuffer_red_field_position;rsize=m->framebuffer_red_mask_size;gpos=m->framebuffer_green_field_position;gsize=m->framebuffer_green_mask_size;bpos=m->framebuffer_blue_field_position;bsize=m->framebuffer_blue_mask_size;
        int sx=(int)(fb_w/(VGA_W*6u)), sy=(int)(fb_h/(VGA_H*8u)); scale=sx<sy?sx:sy; if(scale<1)scale=1; if(scale>6)scale=6;
        off_x=((int)fb_w-VGA_W*6*scale)/2; off_y=((int)fb_h-VGA_H*8*scale)/2;
    }
}

int vga_framebuffer(void){return fb_on;} u32 vga_px_width(void){return fb_w;} u32 vga_px_height(void){return fb_h;} u32 vga_px_bpp(void){return fb_bpp;}
void vga_pixel(int x,int y,u32 rgb){if(!fb_on||x<0||y<0||x>=(int)fb_w||y>=(int)fb_h)return;u32 p=pack_rgb(rgb);u8*d=fb+(u32)y*fb_pitch+(u32)x*((fb_bpp+7)/8);if(fb_bpp==32){*(u32*)d=p;}else if(fb_bpp==24){d[0]=(u8)p;d[1]=(u8)(p>>8);d[2]=(u8)(p>>16);}else if(fb_bpp==16){*(u16*)d=(u16)p;}}
void vga_fill_px(int x,int y,int w,int h,u32 rgb){if(!fb_on||w<=0||h<=0)return;if(x<0){w+=x;x=0;}if(y<0){h+=y;y=0;}if(x+w>(int)fb_w)w=(int)fb_w-x;if(y+h>(int)fb_h)h=(int)fb_h-y;if(w<=0||h<=0)return;u32 p=pack_rgb(rgb),bytes=(fb_bpp+7)/8;for(int yy=0;yy<h;yy++){u8*d=fb+(u32)(y+yy)*fb_pitch+(u32)x*bytes;for(int xx=0;xx<w;xx++,d+=bytes){if(fb_bpp==32)*(u32*)d=p;else if(fb_bpp==24){d[0]=(u8)p;d[1]=(u8)(p>>8);d[2]=(u8)(p>>16);}else if(fb_bpp==16)*(u16*)d=(u16)p;}}}
void vga_cell_rect(int cx,int cy,int cw,int ch,int*x,int*y,int*w,int*h){if(fb_on){*x=off_x+cx*6*scale;*y=off_y+cy*8*scale;*w=cw*6*scale;*h=ch*8*scale;}else{*x=cx;*y=cy;*w=cw;*h=ch;}}

void vga_put(int x,int y,char c,u8 attr){if(x<0||y<0||x>=VGA_W||y>=VGA_H)return;if(!fb_on){VGA[y*VGA_W+x]=(u16)(u8)c|((u16)attr<<8);return;}u32 fg=(attr&15)==vga_hi()?vga_rgb_hi():((attr&15)==vga_dim()?vga_rgb_dim():vga_rgb_fg());u32 bg=vga_rgb_bg();int px=off_x+x*6*scale,py=off_y+y*8*scale;vga_fill_px(px,py,6*scale,8*scale,bg);for(int r=0;r<7;r++){u8 bits=glyph_row(c,r);for(int col=0;col<5;col++)if(bits&(1u<<(4-col)))vga_fill_px(px+col*scale,py+r*scale,scale,scale,fg);}}
void vga_clear(void){if(fb_on){vga_fill_px(0,0,(int)fb_w,(int)fb_h,vga_rgb_bg());return;}u8 a=(u8)(vga_fg()|(vga_bg()<<4));for(int y=0;y<VGA_H;y++)for(int x=0;x<VGA_W;x++)vga_put(x,y,' ',a);}
void vga_text(int x,int y,const char*s,u8 a){while(*s&&x<VGA_W)vga_put(x++,y,*s++,a);}void vga_text_clip(int x,int y,const char*s,int max,u8 a){int n=0;while(*s&&x<VGA_W&&n++<max)vga_put(x++,y,*s++,a);}void vga_fill(int x,int y,int w,int h,char c,u8 a){for(int yy=0;yy<h;yy++)for(int xx=0;xx<w;xx++)vga_put(x+xx,y+yy,c,a);}void vga_hline(int x,int y,int w,char c,u8 a){for(int i=0;i<w;i++)vga_put(x+i,y,c,a);}void vga_vline(int x,int y,int h,char c,u8 a){for(int i=0;i<h;i++)vga_put(x,y+i,c,a);}
void vga_cursor_cell(int x,int y){u8 a=(u8)(vga_hi()|(vga_bg()<<4));vga_put(x,y,'+',a);}
