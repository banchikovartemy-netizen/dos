/* PCOS app layer extension.
 * Keep the mature 0.7 applications intact and wrap them with the new HUD apps.
 * apps/apps.c is intentionally included here and excluded as a standalone TU by
 * the Makefile so all existing static state remains unchanged. */
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

const char *apps_name(AppId a){
    static const char *n[]={
        "FILES","TERMINAL","NOTES","PLAYER","PHOTOS","VIDEO","CALCULATOR",
        "GAMES","BROWSER","ANIMATIONS","MAP","NETWORK","SYSTEM","SETTINGS"
    };
    return a<APP_COUNT?n[a]:"?";
}
void apps_init(u32 magic,u32 mbi_addr){legacy_apps_init(magic,mbi_addr);}
void apps_draw(AppId a){
    if(a==APP_ANIMATIONS){extras_anim_draw_app();return;}
    if(a==APP_MAP){extras_map_draw();return;}
    legacy_apps_draw(a);
}
void apps_key(AppId a,KeyEvent e){
    if(a==APP_ANIMATIONS){extras_anim_key(e);return;}
    if(a==APP_MAP){extras_map_key(e);return;}
    legacy_apps_key(a,e);
}
void apps_mouse(AppId a,int x,int y,u8 buttons,u8 clicked){
    if(a==APP_ANIMATIONS){extras_anim_mouse(x,y,clicked);return;}
    if(a==APP_MAP){extras_map_mouse(x,y,clicked);return;}
    legacy_apps_mouse(a,x,y,buttons,clicked);
}
int apps_tick(AppId a){
    int redraw=0;
    if(a==APP_ANIMATIONS||extras_anim_sidebar())redraw|=extras_anim_tick();
    if(a!=APP_ANIMATIONS&&a!=APP_MAP)redraw|=legacy_apps_tick(a);
    return redraw;
}
