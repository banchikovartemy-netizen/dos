#ifndef PCOS_SB16_H
#define PCOS_SB16_H
#include "types.h"
int sb16_init(void);
int sb16_ready(void);
int sb16_play_u8(const u8 *samples,u32 count,u32 rate);
#endif
