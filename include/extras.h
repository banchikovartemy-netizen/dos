#ifndef PCOS_EXTRAS_H
#define PCOS_EXTRAS_H
#include "types.h"
#include "keyboard.h"
void extras_anim_draw_app(void);
void extras_anim_key(KeyEvent e);
void extras_anim_mouse(int x,int y,u8 clicked);
int extras_anim_tick(void);
int extras_anim_fullscreen(void);
int extras_anim_sidebar(void);
void extras_anim_draw_fullscreen(void);
void extras_anim_draw_sidebar(void);
void extras_map_draw(void);
void extras_map_key(KeyEvent e);
void extras_map_mouse(int x,int y,u8 clicked);
#endif
