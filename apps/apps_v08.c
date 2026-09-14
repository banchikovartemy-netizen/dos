/* PCOS extended app layer.
 * The mature app implementation remains in apps.c; this wrapper adds the
 * minimalist desktop utilities without duplicating the stable core apps. */
#define apps_init legacy_apps_init
#define apps_draw legacy_apps_draw
#define apps_key legacy_apps_key
#define apps_mouse legacy_apps_mouse
#define apps_tick legacy_apps_tick
#define apps_name legacy_apps_name
#include "apps.c"
#undef apps_init
#undef apps_draw
#undef apps_key
#undef apps_mouse
#undef apps_tick
#undef apps_name

#include "extras.h"
#include "rtc.h"
#include "keyboard.h"

/* ------------------------------------------------------------------------- */
/* PCFETCH: a live fastfetch-style view with a procedural rotating ASCII cube. */
static int fetch_active=0;
static u32 fetch_frame=0,fetch_last=0;
static int fetch_cfg_sel=0;
static int fetch_show_cpu=1,fetch_show_ram=1,fetch_show_display=1;
static int fetch_show_network=1,fetch_show_clock=1,fetch_show_input=1;
static int fetch_style=0; /* 0 tactical, 1 clean, 2 diagnostic */
static int fetch_speed=1; /* 0 slow, 1 normal, 2 fast */

static void fp(int x,int y,char c,u8 a){if(x>=20&&x<VGA_W&&y>=4&&y<VGA_H)vga_put(x,y,c,a);}
static void fline(int x0,int y0,int x1,int y1,char c,u8 a){
    int dx=x1>x0?x1-x0:x0-x1,sx=x0<x1?1:-1;
    int dy=-(y1>y0?y1-y0:y0-y1),sy=y0<y1?1:-1,er=dx+dy;
    for(;;){fp(x0,y0,c,a);if(x0==x1&&y0==y1)break;int e2=er*2;if(e2>=dy){er+=dy;x0+=sx;}if(e2<=dx){er+=dx;y0+=sy;}}
}
static void fbox(int x,int y,int w,int h,u8 a){
    if(w<2||h<2)return;vga_hline(x,y,w,'-',a);vga_hline(x,y+h-1,w,'-',a);
    vga_vline(x,y,h,'|',a);vga_vline(x+w-1,y,h,'|',a);
    fp(x,y,'+',a);fp(x+w-1,y,'+',a);fp(x,y+h-1,'+',a);fp(x+w-1,y+h-1,'+',a);
}
static void fetch_cube(int x,int y,u32 f){
    static const int ox[8]={0,1,2,2,1,0,-1,-1};
    static const int oy[8]={-1,-1,0,1,1,1,0,-1};
    int q=(int)(f&7),dx=ox[q]*2,dy=oy[q];
    int x0=x,y0=y+2,x1=x+13,y1=y+9;
    int bx0=x0+dx,by0=y0+dy,bx1=x1+dx,by1=y1+dy;
    fbox(x0,y0,14,8,D());fbox(bx0,by0,14,8,H());
    fline(x0,y0,bx0,by0,'/',D());fline(x1,y0,bx1,by0,'\\',D());
    fline(x0,y1,bx0,by1,'\\',D());fline(x1,y1,bx1,by1,'/',D());
    fp(x+6,y+5,(f&1)?'*':'+',H());
}
static void fetch_pair(int *y,const char*k,const char*v){
    vga_text(43,*y,k,D());vga_text_clip(56,*y,v,21,A());(*y)++;
}
static void draw_fetch(void){
    ui_frame("PCFETCH // LIVE MACHINE PROFILE");
    if(fetch_style==2){
        for(int x=21;x<78;x+=8)vga_vline(x,4,18,'.',D());
        for(int y=5;y<22;y+=4)vga_hline(20,y,58,'.',D());
    }
    vga_text(22,5,fetch_style==0?"[ TACTICAL CORE ]":(fetch_style==1?"[ CLEAN CORE ]":"[ DIAGNOSTIC CORE ]"),H());
    fetch_cube(23,7,fetch_frame);
    vga_text(23,18,"PCOS / i386",H());
    vga_text(23,19,"ASCII HUD",D());
    vga_text(23,21,"ESC OR ENTER = RETURN",D());

    int y=5;char val[48],tmp[20],cpu[13];
    if(fetch_show_cpu){vendor(cpu);fetch_pair(&y,"CPU",cpu);}
    if(fetch_show_ram){kitoa((i32)mem_kb,tmp);kstrncpy(val,tmp,sizeof(val));kstrcat(val," KB",sizeof(val));fetch_pair(&y,"RAM",val);}
    if(fetch_show_display){
        if(vga_framebuffer()){kitoa((i32)vga_px_width(),val);kstrcat(val,"x",sizeof(val));kitoa((i32)vga_px_height(),tmp);kstrcat(val,tmp,sizeof(val));}
        else kstrncpy(val,"VGA 80x25",sizeof(val));
        fetch_pair(&y,"DISPLAY",val);
    }
    if(fetch_show_network){char ip[16];net_get_ip(ip);kstrncpy(val,net_driver(),sizeof(val));kstrcat(val," / ",sizeof(val));kstrcat(val,ip,sizeof(val));fetch_pair(&y,"NETWORK",val);}
    if(fetch_show_input)fetch_pair(&y,"INPUT",keyboard_layout_name());
    if(fetch_show_clock){rtc_format_hud(val,sizeof(val));fetch_pair(&y,"CLOCK",val);}
    fetch_pair(&y,"THEME",vga_theme()==0?"TERMINAL RED":vga_theme()==1?"TERMINAL GREEN":vga_theme()==2?"AMBER":vga_theme()==3?"ICE BLUE":"MONOCHROME");
    fetch_pair(&y,"SHELL","PCOS SHELL");
    vga_text(43,20,"CONFIG",D());vga_text(56,20,"FETCH SETUP APP",H());
}

static const char *onoff(int v){return v?"[ ON ]":"[ OFF ]";}
static void fetch_cfg_row(int row,const char *name,const char *value){
    int y=5+row,sel=row==fetch_cfg_sel;vga_put(21,y,sel?'>':' ',sel?H():D());
    vga_text_clip(23,y,name,22,sel?H():A());vga_text_clip(49,y,value,20,sel?H():D());
}
static void draw_fetch_cfg(void){
    ui_frame("FETCH SETUP // PCFETCH PROFILE");
    fetch_cfg_row(0,"CPU",onoff(fetch_show_cpu));
    fetch_cfg_row(1,"RAM",onoff(fetch_show_ram));
    fetch_cfg_row(2,"DISPLAY",onoff(fetch_show_display));
    fetch_cfg_row(3,"NETWORK",onoff(fetch_show_network));
    fetch_cfg_row(4,"CLOCK",onoff(fetch_show_clock));
    fetch_cfg_row(5,"INPUT LAYOUT",onoff(fetch_show_input));
    fetch_cfg_row(6,"STYLE",fetch_style==0?"TACTICAL":fetch_style==1?"CLEAN":"DIAGNOSTIC");
    fetch_cfg_row(7,"CUBE SPEED",fetch_speed==0?"SLOW":fetch_speed==1?"NORMAL":"FAST");
    vga_hline(21,15,52,'-',D());
    vga_text(21,17,"UP/DOWN SELECT",D());vga_text(21,18,"ENTER / LEFT / RIGHT CHANGE",D());
    vga_text(21,20,"TERMINAL COMMAND",D());vga_text(42,20,"pcfetch",H());
    vga_text(21,22,"LIVE CUBE + SYSTEM TELEMETRY",A());
}
static void fetch_cfg_change(int dir){
    int *toggle=0;
    if(fetch_cfg_sel==0)toggle=&fetch_show_cpu;else if(fetch_cfg_sel==1)toggle=&fetch_show_ram;
    else if(fetch_cfg_sel==2)toggle=&fetch_show_display;else if(fetch_cfg_sel==3)toggle=&fetch_show_network;
    else if(fetch_cfg_sel==4)toggle=&fetch_show_clock;else if(fetch_cfg_sel==5)toggle=&fetch_show_input;
    if(toggle){*toggle=!*toggle;return;}
    if(fetch_cfg_sel==6){fetch_style=(fetch_style+(dir<0?2:1))%3;return;}
    if(fetch_cfg_sel==7){fetch_speed=(fetch_speed+(dir<0?2:1))%3;return;}
}

/* ------------------------------------------------------------------------- */
/* GUIDE: intentionally the only place that contains desktop instructions. */
static void draw_guide(void){
    ui_frame("GUIDE // CONTROL REFERENCE");
    vga_text(21,5,"DESKTOP",H());
    ui_label(21,7,"MOUSE","CLICK APP / CONTROL");
    ui_label(21,8,"TAB","SIDEBAR <-> APP FOCUS");
    ui_label(21,9,"UP / DOWN","SELECT ITEM");
    ui_label(21,10,"ENTER","OPEN / CONFIRM");
    ui_label(21,11,"F12","EN / RU INPUT");
    vga_hline(21,13,52,'-',D());
    vga_text(21,15,"QUICK ACTIONS",H());
    ui_label(21,17,"TERMINAL","pcfetch / help");
    ui_label(21,18,"ANIMATIONS","W HUD / F FULL / S STOP");
    ui_label(21,19,"BROWSER","RUN WITH make run-web");
    ui_label(21,20,"GAMES","DOOM 1 VIA LEGACY DOS");
    ui_label(21,21,"MAP","1..5 MAP LAYERS");
    vga_text(21,23,"NO DESKTOP CLUTTER: HELP LIVES HERE.",D());
}

const char *apps_name(AppId a){
    static const char *n[]={
        "FILES","TERMINAL","NOTES","PLAYER","PHOTOS","VIDEO","CALCULATOR",
        "GAMES","BROWSER","ANIMATIONS","MAP","GUIDE","FETCH SETUP",
        "NETWORK","SYSTEM","SETTINGS"
    };
    return a<APP_COUNT?n[a]:"?";
}

void apps_init(u32 magic,u32 mbi_addr){legacy_apps_init(magic,mbi_addr);}

void apps_draw(AppId a){
    if(a==APP_TERMINAL&&fetch_active){draw_fetch();return;}
    if(a==APP_ANIMATIONS){extras_anim_draw_app();return;}
    if(a==APP_MAP){extras_map_draw();return;}
    if(a==APP_GUIDE){draw_guide();return;}
    if(a==APP_FETCHCFG){draw_fetch_cfg();return;}
    legacy_apps_draw(a);
}

void apps_key(AppId a,KeyEvent e){
    if(!e.pressed)return;
    if(a==APP_TERMINAL){
        if(fetch_active){if(e.special==KEY_ESC||e.special==KEY_ENTER||e.ch=='q'||e.ch=='Q')fetch_active=0;return;}
        if(e.special==KEY_ENTER){
            term_in[term_len]=0;
            int run_fetch=!kstrcmp(term_in,"pcfetch")||!kstrcmp(term_in,"fastfetch")||!kstrcmp(term_in,"fetch");
            int was_help=!kstrcmp(term_in,"help");
            if(run_fetch){term_len=0;term_in[0]=0;fetch_active=1;return;}
            legacy_apps_key(a,e);
            if(was_help)log_line("pcfetch (or fastfetch) // live rotating-cube system view");
            return;
        }
        legacy_apps_key(a,e);return;
    }
    if(a==APP_ANIMATIONS){extras_anim_key(e);return;}
    if(a==APP_MAP){extras_map_key(e);return;}
    if(a==APP_GUIDE)return;
    if(a==APP_FETCHCFG){
        if(e.special==KEY_UP)fetch_cfg_sel=(fetch_cfg_sel+7)%8;
        else if(e.special==KEY_DOWN)fetch_cfg_sel=(fetch_cfg_sel+1)%8;
        else if(e.special==KEY_LEFT)fetch_cfg_change(-1);
        else if(e.special==KEY_RIGHT||e.special==KEY_ENTER)fetch_cfg_change(1);
        return;
    }
    legacy_apps_key(a,e);
}

void apps_mouse(AppId a,int x,int y,u8 buttons,u8 clicked){
    if(!clicked)return;
    if(a==APP_TERMINAL&&fetch_active){fetch_active=0;return;}
    if(a==APP_ANIMATIONS){extras_anim_mouse(x,y,clicked);return;}
    if(a==APP_MAP){extras_map_mouse(x,y,clicked);return;}
    if(a==APP_GUIDE)return;
    if(a==APP_FETCHCFG&&y>=5&&y<13){fetch_cfg_sel=y-5;fetch_cfg_change(1);return;}
    legacy_apps_mouse(a,x,y,buttons,clicked);
}

int apps_tick(AppId a){
    int redraw=0;
    if(a==APP_TERMINAL&&fetch_active){
        u32 hz=timer_hz(),t=timer_ticks();u32 div=fetch_speed==0?4u:(fetch_speed==1?8u:14u);u32 step=hz/div;if(step<1)step=1;
        if(t-fetch_last>=step){fetch_last=t;fetch_frame++;redraw=1;}
        return redraw;
    }
    if(a==APP_ANIMATIONS||extras_anim_sidebar())redraw|=extras_anim_tick();
    if(a!=APP_ANIMATIONS&&a!=APP_MAP&&a!=APP_GUIDE&&a!=APP_FETCHCFG)redraw|=legacy_apps_tick(a);
    return redraw;
}
