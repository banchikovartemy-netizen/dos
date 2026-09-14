#include "ui.h"
#include "vga.h"
#include "apps.h"
#include "keyboard.h"
#include "rtc.h"
#include "extras.h"

#define SIDE_W 18
#define SIDE_Y 2

static u8 attr(u8 fg){return(u8)(fg|(vga_bg()<<4));}

static const char *app_icon(AppId a){
    static const char *icons[]={
        "[]",">_","N=","|>","##","V>","+-","G*",
        "@>","<> ","M:","??","F>","N#","S:","::"
    };
    return a<APP_COUNT?icons[a]:"??";
}

static void topbar(void){
    u8 a=attr(vga_fg()),d=attr(vga_dim()),h=attr(vga_hi());
    char t[40];rtc_format_hud(t,sizeof(t));
    vga_text(1,0,"PCOS",h);
    vga_text(6,0,"//",d);
    vga_text(9,0,keyboard_layout_name(),a);
    vga_text_clip(49,0,t,30,a);
    vga_hline(0,1,VGA_W,'-',d);
}

void ui_frame(const char*title){
    u8 d=attr(vga_dim()),h=attr(vga_hi());
    vga_text(20,2,"//",d);
    vga_text_clip(23,2,title,54,h);
    vga_hline(20,3,58,'-',d);
}

void ui_label(int x,int y,const char*l,const char*v){
    vga_text(x,y,l,attr(vga_dim()));
    vga_text_clip(x+14,y,v,42,attr(vga_fg()));
}

static void sidebar(AppId app){
    u8 a=attr(vga_fg()),d=attr(vga_dim()),h=attr(vga_hi());
    vga_vline(SIDE_W,2,VGA_H-2,':',d);
    for(int i=0;i<APP_COUNT;i++){
        int y=SIDE_Y+i,sel=i==(int)app;
        vga_put(0,y,sel?'>':' ',sel?h:d);
        vga_put(1,y,'[',sel?h:d);
        vga_text_clip(2,y,app_icon((AppId)i),2,sel?h:d);
        vga_put(4,y,']',sel?h:d);
        vga_text_clip(6,y,apps_name((AppId)i),11,sel?h:a);
        vga_put(17,y,sel?'<':' ',sel?h:d);
    }
}

void ui_draw(AppId app){
    vga_clear();
    topbar();
    if(app==APP_ANIMATIONS&&extras_anim_fullscreen()){
        extras_anim_draw_fullscreen();
        return;
    }
    sidebar(app);
    apps_draw(app);
}
