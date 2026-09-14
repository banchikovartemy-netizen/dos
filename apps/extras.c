#include "extras.h"
#include "ui.h"
#include "vga.h"
#include "timer.h"
#include "lib.h"

#define CX 20
static u8 A(void){return(u8)(vga_fg()|(vga_bg()<<4));}
static u8 D(void){return(u8)(vga_dim()|(vga_bg()<<4));}
static u8 H(void){return(u8)(vga_hi()|(vga_bg()<<4));}

static int anim_sel=0;       /* 0..9 */
static int anim_mode=0;      /* 0 inline, 1 sidebar, 2 fullscreen */
static u32 anim_frame=0,anim_last=0;
static const char *anim_names[10]={
    "SPIN CUBE","TARGET LOCK","RADAR SWEEP","DNA HELIX","ORBIT CORE",
    "WAVEFORM","MATRIX RAIN","DATA TUNNEL","SATELLITE","REACTOR"
};

static void p(int x,int y,char c,u8 a){if(x>=0&&x<VGA_W&&y>=0&&y<VGA_H)vga_put(x,y,c,a);}
static void line(int x0,int y0,int x1,int y1,char c,u8 a){
    int dx=x1>x0?x1-x0:x0-x1,sx=x0<x1?1:-1,dy=-(y1>y0?y1-y0:y0-y1),sy=y0<y1?1:-1,er=dx+dy;
    for(;;){p(x0,y0,c,a);if(x0==x1&&y0==y1)break;int e2=er*2;if(e2>=dy){er+=dy;x0+=sx;}if(e2<=dx){er+=dx;y0+=sy;}}
}
static void box(int x,int y,int w,int h,u8 a){if(w<2||h<2)return;vga_hline(x,y,w,'-',a);vga_hline(x,y+h-1,w,'-',a);vga_vline(x,y,h,'|',a);vga_vline(x+w-1,y,h,'|',a);p(x,y,'+',a);p(x+w-1,y,'+',a);p(x,y+h-1,'+',a);p(x+w-1,y+h-1,'+',a);}
static int imin(int a,int b){return a<b?a:b;}static int imax(int a,int b){return a>b?a:b;}

static void anim_cube(int x,int y,int w,int h,u32 f){
    int s=imin(w/3,h/2);if(s<3)s=3;int cx=x+w/2,cy=y+h/2;int q=(int)(f&7);int ox=(q<4?q:7-q)-2,oy=((q+2)&3)-1;
    int x0=cx-s,y0=cy-s/2,x1=cx+s,y1=cy+s/2;box(x0,y0,x1-x0+1,y1-y0+1,A());
    int bx0=x0+ox*2,by0=y0+oy,bx1=x1+ox*2,by1=y1+oy;box(bx0,by0,bx1-bx0+1,by1-by0+1,H());
    line(x0,y0,bx0,by0,'/',D());line(x1,y0,bx1,by0,'\\',D());line(x0,y1,bx0,by1,'\\',D());line(x1,y1,bx1,by1,'/',D());
}
static void anim_target(int x,int y,int w,int h,u32 f){
    int cx=x+w/2,cy=y+h/2,m=imin(w/2,h);for(int r=2;r<m;r+=3){int pulse=(int)((f/2+r)%4);box(cx-r,cy-r/2,2*r+1,r+1,pulse?D():H());}
    vga_hline(x,cy,w,'-',D());vga_vline(cx,y,h,'|',D());p(cx,cy,(f&1)?'+':'X',H());
    if(w>18){vga_text(x+2,y+1,"TARGET",D());vga_text(x+2,y+2,(f&4)?"LOCKED":"SCANNING",H());}
}
static void anim_radar(int x,int y,int w,int h,u32 f){
    int cx=x+w/2,cy=y+h/2,rx=imax(3,imin(w/2-2,h-1));
    for(int yy=-rx/2;yy<=rx/2;yy++)for(int xx=-rx;xx<=rx;xx++){int v=xx*xx*4+yy*yy*16-rx*rx*4;if(v>-rx*8&&v<rx*8)p(cx+xx,cy+yy,'.',D());}
    static const int dx[8]={1,1,0,-1,-1,-1,0,1},dy[8]={0,1,1,1,0,-1,-1,-1};int d=(int)(f&7);line(cx,cy,cx+dx[d]*rx,cy+dy[d]*(rx/2),'/',H());p(cx,cy,'O',H());
    int blipx=cx-rx/3,blipy=cy+rx/5;p(blipx,blipy,(f&2)?'*':'o',H());
}
static void anim_helix(int x,int y,int w,int h,u32 f){
    static const int wave[16]={0,1,2,3,4,3,2,1,0,-1,-2,-3,-4,-3,-2,-1};int cx=x+w/2,amp=imin(6,w/4);
    for(int yy=0;yy<h;yy++){int a=wave[(yy+(int)f)&15]*amp/4,b=wave[(yy+(int)f+8)&15]*amp/4;int xa=cx+a,xb=cx+b;p(xa,y+yy,'@',H());p(xb,y+yy,'o',A());if((yy+f)%3==0)line(xa,y+yy,xb,y+yy,'-',D());}
}
static void anim_orbit(int x,int y,int w,int h,u32 f){
    int cx=x+w/2,cy=y+h/2,rx=imax(3,w/3),ry=imax(2,h/3);for(int i=0;i<16;i++){static const int wx[16]={8,7,6,3,0,-3,-6,-7,-8,-7,-6,-3,0,3,6,7};static const int wy[16]={0,2,3,4,4,4,3,2,0,-2,-3,-4,-4,-4,-3,-2};p(cx+wx[i]*rx/8,cy+wy[i]*ry/4,'.',D());}
    int k=(int)(f&15);static const int wx[16]={8,7,6,3,0,-3,-6,-7,-8,-7,-6,-3,0,3,6,7};static const int wy[16]={0,2,3,4,4,4,3,2,0,-2,-3,-4,-4,-4,-3,-2};p(cx,cy,'@',H());p(cx+wx[k]*rx/8,cy+wy[k]*ry/4,'*',H());p(cx+wx[(k+8)&15]*rx/8,cy+wy[(k+8)&15]*ry/4,'o',A());
}
static void anim_wave(int x,int y,int w,int h,u32 f){
    static const int wave[24]={0,1,2,3,2,1,0,-1,-2,-3,-2,-1,0,2,4,2,0,-2,-4,-2,0,1,0,-1};int cy=y+h/2,amp=imax(1,h/3);vga_hline(x,cy,w,'-',D());
    for(int xx=0;xx<w;xx++){int yy=cy+wave[(xx+(int)f)%24]*amp/4;p(x+xx,yy,(xx+(int)f)%7?'*':'@',H());}
}
static void anim_rain(int x,int y,int w,int h,u32 f){
    static const char glyph[]="01ABCDEF#@+";for(int xx=0;xx<w;xx++){int head=(xx*7+(int)f)%(h+7)-3;for(int k=0;k<4;k++){int yy=head-k;if(yy>=0&&yy<h)p(x+xx,y+yy,glyph[(xx+k+(int)f)%10],k?D():H());}}
}
static void anim_tunnel(int x,int y,int w,int h,u32 f){
    int levels=imin(w/6,h/2);for(int i=0;i<levels;i++){int t=(i+(int)(f/2))%levels;int ww=w-t*4,hh=h-t*2;if(ww>3&&hh>2)box(x+(w-ww)/2,y+(h-hh)/2,ww,hh,t==0?H():D());}
    p(x+w/2,y+h/2,'+',H());
}
static void anim_sat(int x,int y,int w,int h,u32 f){
    for(int i=0;i<w;i+=7)p(x+i,y+(i*3+(int)f)%h,'.',D());int sx=x+(int)(f%(u32)(w+8))-4,sy=y+h/2+((int)(f/4)%3)-1;
    if(sx>=x&&sx<x+w)p(sx,sy,'O',H());if(sx-1>=x)p(sx-1,sy,'[',A());if(sx+1<x+w)p(sx+1,sy,']',A());if(sx-3>=x){p(sx-2,sy,'=',D());p(sx-3,sy,'=',D());}if(sx+3<x+w){p(sx+2,sy,'=',D());p(sx+3,sy,'=',D());}
}
static void anim_reactor(int x,int y,int w,int h,u32 f){
    int cx=x+w/2,cy=y+h/2,r=imin(w/3,h/2-1);if(r<2)r=2;static const int dx[8]={1,1,0,-1,-1,-1,0,1},dy[8]={0,1,1,1,0,-1,-1,-1};
    int rot=(int)(f&7);for(int i=0;i<8;i++){int d=(i+rot)&7;line(cx,cy,cx+dx[d]*r,cy+dy[d]*(r/2),(i&1)?'/':'\\',i&1?A():D());}p(cx,cy,(f&1)?'@':'O',H());if(w>18)vga_text(x+2,y+1,(f&4)?"CORE STABLE":"CORE CHARGING",H());
}
static void draw_anim_region(int x,int y,int w,int h,int id,u32 f){
    if(w<4||h<3)return;switch(id){case 0:anim_cube(x,y,w,h,f);break;case 1:anim_target(x,y,w,h,f);break;case 2:anim_radar(x,y,w,h,f);break;case 3:anim_helix(x,y,w,h,f);break;case 4:anim_orbit(x,y,w,h,f);break;case 5:anim_wave(x,y,w,h,f);break;case 6:anim_rain(x,y,w,h,f);break;case 7:anim_tunnel(x,y,w,h,f);break;case 8:anim_sat(x,y,w,h,f);break;default:anim_reactor(x,y,w,h,f);break;}}

void extras_anim_draw_app(void){
    ui_frame("ANIMATIONS // ASCII MOTION LIBRARY");vga_text(CX,4,"SELECT",D());
    for(int i=0;i<10;i++){int yy=5+i;vga_put(CX,yy,i==anim_sel?'>':' ',i==anim_sel?H():A());vga_text_clip(CX+2,yy,anim_names[i],16,i==anim_sel?H():A());}
    box(CX+20,4,35,13,D());draw_anim_region(CX+22,5,31,11,anim_sel,anim_frame);
    vga_text(CX,17,"ENTER INLINE   W LEFT HUD   F FULLSCREEN   S STOP",D());
    vga_text(CX,19,"MODE",D());vga_text(CX+7,19,anim_mode==2?"FULLSCREEN":(anim_mode==1?"LEFT HUD":"INLINE"),H());
    vga_text(CX,21,"10 LOW-COST PROCEDURAL ASCII ANIMATIONS",A());
}
void extras_anim_key(KeyEvent e){if(!e.pressed)return;if(e.special==KEY_ESC&&anim_mode==2){anim_mode=0;return;}if(e.special==KEY_UP){anim_sel=(anim_sel+9)%10;return;}if(e.special==KEY_DOWN){anim_sel=(anim_sel+1)%10;return;}if(e.special==KEY_ENTER){anim_mode=0;return;}if(e.ch=='w'||e.ch=='W'){anim_mode=1;return;}if(e.ch=='f'||e.ch=='F'){anim_mode=2;return;}if(e.ch=='s'||e.ch=='S'){anim_mode=0;return;}if(e.ch>='0'&&e.ch<='9'){anim_sel=e.ch=='0'?9:e.ch-'1';}}
void extras_anim_mouse(int x,int y,u8 clicked){if(!clicked)return;if(y>=5&&y<15&&x>=CX&&x<CX+19){anim_sel=y-5;return;}if(y==17){if(x<CX+18)anim_mode=0;else if(x<CX+36)anim_mode=1;else anim_mode=2;}}
int extras_anim_tick(void){u32 hz=timer_hz(),t=timer_ticks(),step=hz/8;if(step<1)step=1;if(t-anim_last>=step){anim_last=t;anim_frame++;return 1;}return 0;}
int extras_anim_fullscreen(void){return anim_mode==2;}int extras_anim_sidebar(void){return anim_mode==1;}
void extras_anim_draw_sidebar(void){box(1,19,15,5,D());draw_anim_region(2,20,13,3,anim_sel,anim_frame);}
void extras_anim_draw_fullscreen(void){vga_text(2,2,"ANIMATION//",D());vga_text(13,2,anim_names[anim_sel],H());vga_text(59,2,"ESC=EXIT",D());draw_anim_region(2,4,76,19,anim_sel,anim_frame);}

/* ------------------------------------------------------------------------- */
/* Offline schematic world map. The geometry is intentionally HUD-stylized. */
static int map_mode=0;
static const char *map_modes[5]={"COUNTRIES","ROADS","RAIL","CITIES","TIME ZONES"};
#define MX 21
#define MY 7
#define MW 56
#define MH 14
static void mp(int x,int y,char c,u8 a){if(x>=0&&x<MW&&y>=0&&y<MH)vga_put(MX+x,MY+y,c,a);}
static int inside_ellipse(int x,int y,int cx,int cy,int rx,int ry){int dx=x-cx,dy=y-cy;return dx*dx*ry*ry+dy*dy*rx*rx<=rx*rx*ry*ry;}
static int land(int x,int y){
    if(inside_ellipse(x,y,9,3,8,3))return 1;                 /* N America */
    if(inside_ellipse(x,y,15,9,4,5))return 1;                /* S America */
    if(inside_ellipse(x,y,28,3,4,2))return 1;                /* Europe */
    if(inside_ellipse(x,y,30,8,5,5))return 1;                /* Africa */
    if(inside_ellipse(x,y,40,4,12,4))return 1;               /* Asia */
    if(inside_ellipse(x,y,47,11,5,2))return 1;               /* Australia */
    if(inside_ellipse(x,y,5,0,3,1))return 1;                 /* Greenland */
    return 0;
}
static void map_base(void){
    for(int y=0;y<MH;y++)for(int x=0;x<MW;x++){char c=land(x,y)?'#':((x+y)%17==0?'.':' ');mp(x,y,c,land(x,y)?D():(u8)(vga_bg()<<4));}
    vga_hline(MX,MY+MH,MW,'-',D());
}
static void ml(int x0,int y0,int x1,int y1,char c,u8 a){line(MX+x0,MY+y0,MX+x1,MY+y1,c,a);}
static void label(int x,int y,const char*s){vga_text(MX+x,MY+y,s,H());}
static void map_countries(void){
    label(4,2,"CAN");label(7,4,"USA");label(12,8,"BRA");label(25,2,"UK");label(27,3,"FRA");label(30,2,"POL");label(33,2,"RUS");label(28,7,"EGY");label(29,10,"RSA");label(37,5,"IND");label(43,4,"CHN");label(50,4,"JPN");label(45,11,"AUS");
}
static void map_roads(void){
    ml(3,4,13,4,'=',H());ml(7,2,10,6,'=',A());ml(12,7,16,12,'=',H());ml(24,3,34,3,'=',H());ml(28,3,31,8,'=',A());ml(34,3,47,4,'=',H());ml(38,5,45,7,'=',A());ml(45,10,51,11,'=',H());
    label(1,12,"ROAD GRID // PRIMARY CORRIDORS");
}
static void map_rail(void){
    ml(4,3,12,5,'-',H());ml(25,2,34,3,'-',A());ml(33,3,49,4,'-',H());ml(37,5,43,7,'-',A());ml(27,4,29,9,'-',H());ml(44,4,50,4,'-',H());
    for(int x=4;x<50;x+=4)if(land(x,4))mp(x,4,'+',H());label(1,12,"RAIL NET // INTERCITY TRUNK LINES");
}
static void city(int x,int y,const char*s){mp(x,y,'*',H());if(s)vga_text_clip(MX+x+1,MY+y,s,3,A());}
static void map_cities(void){
    city(8,4,"NYC");city(4,4,"LAX");city(14,9,"SAO");city(25,3,"LON");city(27,4,"PAR");city(31,3,"BER");city(34,3,"MOS");city(31,6,"IST");city(34,7,"DXB");city(38,6,"DEL");city(44,4,"BEI");city(50,4,"TYO");city(43,8,"SIN");city(48,11,"SYD");city(29,10,"JNB");
}
static void map_timezones(void){
    for(int x=2;x<MW;x+=4){for(int y=0;y<MH;y++)mp(x,y,'|',(x==30)?H():D());}
    label(1,12,"UTC-12      UTC-6       UTC       UTC+6      UTC+12");
}
void extras_map_draw(void){
    ui_frame("WORLD MAP // OFFLINE TERMINATOR ATLAS");
    vga_text(CX,4,"1 COUNTRIES  2 ROADS  3 RAIL  4 CITIES  5 TIME",D());
    vga_text(CX,5,"MODE::",D());vga_text(CX+7,5,map_modes[map_mode],H());
    map_base();if(map_mode==0)map_countries();else if(map_mode==1)map_roads();else if(map_mode==2)map_rail();else if(map_mode==3)map_cities();else map_timezones();
    vga_text(CX,22,"SCHEMATIC OFFLINE MAP // HUD DATASET // NOT FOR NAVIGATION",D());
}
void extras_map_key(KeyEvent e){if(!e.pressed)return;if(e.ch>='1'&&e.ch<='5')map_mode=e.ch-'1';else if(e.special==KEY_LEFT)map_mode=(map_mode+4)%5;else if(e.special==KEY_RIGHT)map_mode=(map_mode+1)%5;}
void extras_map_mouse(int x,int y,u8 clicked){if(!clicked)return;if(y==4&&x>=CX){int r=(x-CX)/11;if(r<0)r=0;if(r>4)r=4;map_mode=r;}}
