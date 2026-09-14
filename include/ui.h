#ifndef PCOS_UI_H
#define PCOS_UI_H
#include "apps.h"
void ui_draw(AppId app);
void ui_frame(const char *title);
void ui_label(int x,int y,const char *label,const char *value);
#endif
