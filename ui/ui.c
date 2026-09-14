#include "ui.h"
#include "vga.h"
#include "apps.h"
#define SIDE_W 17
static u8 attr(u8 fg){return(u8)(fg|(vga_bg()<<4));}
void ui_frame(const char*title){vga_text(19,1,"[ ",attr(vga_dim()));vga_text_clip(21,1,title,44,attr(vga_hi()));vga_text(66,1," ]",attr(vga_dim()));vga_hline(19,2,59,'-',attr(vga_dim()));}
void ui_label(int x,int y,const char*l,const char*v){vga_text(x,y,l,attr(vga_dim()));vga_text_clip(x+14,y,v,42,attr(vga_fg()));}
void ui_draw(AppId app){
    vga_clear();u8 a=attr(vga_fg()),d=attr(vga_dim()),h=attr(vga_hi());
    vga_hline(0,0,VGA_W,'=',d);
    vga_text(2,0,"PCOS // КИБЕРТЕРМИНАЛ",h);
    vga_text(61,0,"СИСТЕМА::ГОТОВА",a);
    vga_vline(SIDE_W,1,24,'|',d);
    vga_text(2,2,"ПРИЛОЖЕНИЯ",d);
    vga_hline(1,3,15,'-',d);
    for(int i=0;i<APP_COUNT;i++){
        int y=4+i;
        if(i==(int)app){vga_put(1,y,'>',h);vga_text_clip(3,y,apps_name((AppId)i),13,h);}
        else vga_text_clip(3,y,apps_name((AppId)i),13,a);
    }
    vga_text(2,17,"ВВЕРХ/ВНИЗ",d);
    vga_text(2,18,"F1..F11 БЫСТРО",d);
    vga_text(2,20,"ASCII//HUD",d);
    vga_text(2,21,"ВВОД//PS2",a);
    vga_text(2,23,"БЕЗ КОМПОЗИТОРА",d);
    apps_draw(app);
}
