#include "dos86.h"
#include "lib.h"
#include "fs.h"
#include "vga.h"
#include "timer.h"
#include "sb16.h"

#define MEMSZ (1024u*1024u)
#define TEXT_W 80
#define TEXT_H 25
#define MAX_HANDLES 12
#define KEYQ 16

/* 8086 flags */
enum { F_CF=0x0001,F_PF=0x0004,F_AF=0x0010,F_ZF=0x0040,F_SF=0x0080,
       F_TF=0x0100,F_IF=0x0200,F_DF=0x0400,F_OF=0x0800 };

typedef struct { const u8 *data; u32 size,pos; int used; } DFile;
typedef struct {
 u16 ax,bx,cx,dx,si,di,bp,sp,ip,cs,ds,es,ss,fl;
 u16 seg_override; u8 rep; u8 running,waiting,video_mode;
 u8 key_ascii[KEYQ],key_scan[KEYQ],kh,kt;
 u16 alloc_seg;
 u32 insns;
 DFile fh[MAX_HANDLES];
 char name[48],status[96],out[512]; u32 outn;
 char text[TEXT_W*TEXT_H]; u8 tcx,tcy;
 u8 pal[256][3],dac_index,dac_comp;
 u8 seq_idx,seq[8],crtc_idx,crtc[32],gc_idx,gc[16];
 u8 planes[4][65536];
