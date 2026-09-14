#ifndef PCOS_MOUSE_H
#define PCOS_MOUSE_H
#include "types.h"
typedef struct { i8 dx,dy; u8 buttons; u8 moved; u8 clicked; } MouseEvent;
void mouse_init(void);
MouseEvent mouse_poll(void);
int mouse_available(void);
#endif
