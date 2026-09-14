#ifndef PCOS_DOS86_H
#define PCOS_DOS86_H
#include "types.h"
#include "keyboard.h"

/* DOS86 is PCOS' low-power real-mode DOS compatibility runtime.
 * It executes 8086/80186 code in a sandboxed 1 MiB address space and
 * provides a small BIOS/DOS personality plus a virtual VGA adapter. */
int dos86_start(const u8 *data,u32 size,const char *name);
void dos86_stop(void);
int dos86_active(void);
int dos86_step(u32 budget);
void dos86_key(KeyEvent e);
void dos86_draw(int cx,int cy,int cw,int ch);
const char *dos86_status(void);
const char *dos86_level(void);

/* Synchronous compatibility helper used by the terminal. Supports COM/MZ. */
int dos86_run(const u8 *data,u32 size,const char *name,char *out,u32 outn);
int dos86_run_com(const u8 *data,u32 size,char *out,u32 outn);
#endif
