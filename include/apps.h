#ifndef PCOS_APPS_H
#define PCOS_APPS_H
#include "types.h"
#include "keyboard.h"
typedef enum { APP_FILES=0,APP_TERMINAL,APP_NOTES,APP_PLAYER,APP_PHOTOS,APP_VIDEO,APP_CALCULATOR,APP_GAMES,APP_NETWORK,APP_SYSTEM,APP_SETTINGS,APP_COUNT } AppId;
void apps_init(u32 magic,u32 mbi_addr);
void apps_draw(AppId app);
void apps_key(AppId app,KeyEvent e);
int apps_tick(AppId app);
const char *apps_name(AppId app);
#endif
