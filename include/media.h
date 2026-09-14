#ifndef PCOS_MEDIA_H
#define PCOS_MEDIA_H
#include "types.h"
int bmp_draw(const u8 *data,u32 size,int cell_x,int cell_y,int cell_w,int cell_h);
int avi_frame_count(const u8 *data,u32 size);
int avi_draw_frame(const u8 *data,u32 size,int frame,int cell_x,int cell_y,int cell_w,int cell_h);
int wav_play(const u8 *data,u32 size);
int wav_backend_ready(void);
#endif
