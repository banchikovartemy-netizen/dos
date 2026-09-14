#ifndef PCOS_KEYBOARD_H
#define PCOS_KEYBOARD_H
#include "types.h"
typedef struct { char ch; u8 pressed; u8 special; u8 ctrl; u8 alt; u8 shift; } KeyEvent;
enum {KEY_NONE=0,KEY_ENTER=1,KEY_BACKSPACE=2,KEY_TAB=3,KEY_ESC=4,KEY_F1=10,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12,KEY_UP=30,KEY_DOWN,KEY_LEFT,KEY_RIGHT};
KeyEvent keyboard_poll(void);
#endif
