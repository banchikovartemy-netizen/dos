#include "ui.h"
#include "vga.h"
#include "apps.h"
#include "keyboard.h"
#include "rtc.h"
#include "extras.h"
#define SIDE_W 17
static u8 attr(u8 fg){return(u8)(fg|(vga_bg()<<4));}
static void topbar(void){
    u8 a=attr(vga_fg()),d=attr(vga_dim()),h=attr(vga_hi());char t[40];rtc_format_hud(t,sizeof(t));
    vga_hline(0,0,VGA_W,'=',d);vga_text(2,0,"PCOS // HUD",h);vga_text_clip(44,0,t,34,a);
}
void ui_frame(const char*title){vga_text(19,1,"[ ",attr(vga_dim()));vga_text_clip(21,1,title,44,attr(vga_hi()));vga_text(66,1," ]",attr(vga_dim()));vga_hline(19,2,59,'-',attr(vga_dim()));}
void ui_label(int x,int y,const char*l,const char*v){vga_text(x,y,l,attr(vga_dim()));vga_text_clip(x+14,y,v,42,attr(vga_fg()));}
void ui_draw(AppId app){
    vga_clear();topbar();
    if(app==APP_ANIMATIONS&&extras_anim_fullscreen()){extras_anim_draw_fullscreen();return;}
    u8 a=attr(vga_fg()),d=attr(vga_dim()),h=attr(vga_hi());
    vga_vline(SIDE_W,1,24,'|',d);vga_text(2,2,"APPS",d);vga_hline(1,3,15,'-',d);
    for(int i=0;i<APP_COUNT;i++){
        int y=4+i;
        if(i==(int)app){vga_put(1,y,'>',h);vga_text_clip(3,y,apps_name((AppId)i),13,h);}
        else vga_text_clip(3,y,apps_name((AppId)i),13,a);
    }
    if(extras_anim_sidebar()){
        vga_text(2,18,"ANIM HUD",d);extras_anim_draw_sidebar();
    }else{
        vga_text(2,18,"TAB MENU/APP",d);vga_text(2,19,"MOUSE CLICK",d);vga_text(2,20,"F12 RU/EN",d);vga_text(2,21,"LAYOUT//",a);vga_text(11,21,keyboard_layout_name(),h);vga_text(2,23,"ASCII//HUD",d);
    }
    apps_draw(app);
}
