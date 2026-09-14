 u16 dma_addr,dma_count,sb_block; u8 dma_page,dma_ff,dma_mask;
 u32 sb_rate; u8 sb_cmd,sb_need,sb_argn,sb_args[4],sb_resp[8],sb_rh,sb_rt,sb_reset,last_scan;
} Cpu;

typedef struct { int isreg; u8 reg; u16 seg,off; } EA;
static u8 mem[MEMSZ];
static Cpu c;

static u32 lin(u16 s,u16 o){ return (((u32)s<<4)+o)&0xFFFFFu; }
static u8 mr8(u16 s,u16 o){u32 a=lin(s,o);if(a>=0xA0000&&a<0xB0000&&c.video_mode==0x13&&!(c.seq[4]&0x08))return c.planes[c.gc[4]&3][(a-0xA0000)&0xFFFF];return mem[a];}
static u16 mr16(u16 s,u16 o){return (u16)(mr8(s,o)|((u16)mr8(s,(u16)(o+1))<<8));}
static void mw8(u16 s,u16 o,u8 v){u32 a=lin(s,o);if(a>=0xA0000&&a<0xB0000&&c.video_mode==0x13&&!(c.seq[4]&0x08)){u16 q=(u16)(a-0xA0000);u8 mask=c.seq[2]?c.seq[2]:0x0F;for(int p=0;p<4;p++)if(mask&(1u<<p))c.planes[p][q]=v;return;}mem[a]=v;}
static void mw16(u16 s,u16 o,u16 v){mw8(s,o,(u8)v);mw8(s,(u16)(o+1),(u8)(v>>8));}
static u8 fetch8(void){ return mr8(c.cs,c.ip++); }
static u16 fetch16(void){ u16 v=mr16(c.cs,c.ip);c.ip+=2;return v; }
static u16 *r16(int r){ switch(r&7){case 0:return &c.ax;case 1:return &c.cx;case 2:return &c.dx;case 3:return &c.bx;case 4:return &c.sp;case 5:return &c.bp;case 6:return &c.si;default:return &c.di;} }
static u8 r8(int r){ u16 *p=r16(r&3);return (r&4)?(u8)(*p>>8):(u8)*p; }
static void setr8(int r,u8 v){u16*p=r16(r&3);if(r&4)*p=(u16)((*p&255)|((u16)v<<8));else *p=(u16)((*p&0xFF00)|v);}
static u16 sreg(int r){switch(r&3){case 0:return c.es;case 1:return c.cs;case 2:return c.ss;default:return c.ds;}}
static void setsreg(int r,u16 v){switch(r&3){case 0:c.es=v;break;case 1:c.cs=v;break;case 2:c.ss=v;break;default:c.ds=v;break;}}
static void push(u16 v){c.sp-=2;mw16(c.ss,c.sp,v);} static u16 pop(void){u16 v=mr16(c.ss,c.sp);c.sp+=2;return v;}
static int parity(u8 v){v^=v>>4;v&=15;return !((0x6996>>v)&1);}
static void logicf(u32 r,int bits){u32 m=bits==8?0xFFu:0xFFFFu,s=bits==8?0x80u:0x8000u;c.fl&=~(F_CF|F_OF|F_AF|F_ZF|F_SF|F_PF);if(!(r&m))c.fl|=F_ZF;if(r&s)c.fl|=F_SF;if(parity((u8)r))c.fl|=F_PF;}
static u16 alu16(int op,u16 a,u16 b){u32 r=0,cin=(c.fl&F_CF)?1:0;c.fl&=~(F_CF|F_OF|F_AF|F_ZF|F_SF|F_PF);switch(op){case 0:r=(u32)a+b;break;case 1:r=a|b;break;case 2:r=(u32)a+b+cin;break;case 3:r=(u32)a-b-cin;break;case 4:r=a&b;break;case 5:r=(u32)a-b;break;case 6:r=a^b;break;default:r=(u32)a-b;break;}if(op==0||op==2){if(r>0xFFFF)c.fl|=F_CF;if((~(a^b)&(a^(u16)r)&0x8000))c.fl|=F_OF;if(((a^b^(u16)r)&0x10))c.fl|=F_AF;}else if(op==3||op==5||op==7){u32 bb=(u32)b+((op==3)?cin:0);if((u32)a<bb)c.fl|=F_CF;if(((a^b)&(a^(u16)r)&0x8000))c.fl|=F_OF;if(((a^b^(u16)r)&0x10))c.fl|=F_AF;}u16 x=(u16)r;if(!x)c.fl|=F_ZF;if(x&0x8000)c.fl|=F_SF;if(parity((u8)x))c.fl|=F_PF;return x;}
static u8 alu8(int op,u8 a,u8 b){u16 r=0,cin=(c.fl&F_CF)?1:0;c.fl&=~(F_CF|F_OF|F_AF|F_ZF|F_SF|F_PF);switch(op){case 0:r=(u16)a+b;break;case 1:r=a|b;break;case 2:r=(u16)a+b+cin;break;case 3:r=(u16)a-b-cin;break;case 4:r=a&b;break;case 5:r=(u16)a-b;break;case 6:r=a^b;break;default:r=(u16)a-b;break;}if(op==0||op==2){if(r>255)c.fl|=F_CF;if((~(a^b)&(a^(u8)r)&0x80))c.fl|=F_OF;if((a^b^(u8)r)&0x10)c.fl|=F_AF;}else if(op==3||op==5||op==7){u16 bb=(u16)b+((op==3)?cin:0);if(a<bb)c.fl|=F_CF;if(((a^b)&(a^(u8)r)&0x80))c.fl|=F_OF;if((a^b^(u8)r)&0x10)c.fl|=F_AF;}u8 x=(u8)r;if(!x)c.fl|=F_ZF;if(x&0x80)c.fl|=F_SF;if(parity(x))c.fl|=F_PF;return x;}
static void ea_decode(u8 m,EA*e){u8 mod=m>>6,rm=m&7;if(mod==3){e->isreg=1;e->reg=rm;return;}e->isreg=0;u16 base=0;int bp=0;switch(rm){case 0:base=c.bx+c.si;break;case 1:base=c.bx+c.di;break;case 2:base=c.bp+c.si;bp=1;break;case 3:base=c.bp+c.di;bp=1;break;case 4:base=c.si;break;case 5:base=c.di;break;case 6:if(mod==0){base=fetch16();bp=0;}else{base=c.bp;bp=1;}break;default:base=c.bx;break;}if(mod==1)base=(u16)(base+(i8)fetch8());else if(mod==2)base=(u16)(base+fetch16());e->off=base;e->seg=c.seg_override?c.seg_override:(bp?c.ss:c.ds);}
static u8 ea8(EA*e){return e->isreg?r8(e->reg):mr8(e->seg,e->off);}static u16 ea16(EA*e){return e->isreg?*r16(e->reg):mr16(e->seg,e->off);}static void sea8(EA*e,u8 v){if(e->isreg)setr8(e->reg,v);else mw8(e->seg,e->off,v);}static void sea16(EA*e,u16 v){if(e->isreg)*r16(e->reg)=v;else mw16(e->seg,e->off,v);}
static void outch(char ch){if(c.outn+1<sizeof(c.out)){c.out[c.outn++]=ch;c.out[c.outn]=0;}if(ch=='\r')c.tcx=0;else if(ch=='\n'){c.tcx=0;if(++c.tcy>=TEXT_H)c.tcy=TEXT_H-1;}else{u32 q=(u32)c.tcy*TEXT_W+c.tcx;c.text[q]=ch;mem[0xB8000+q*2]=(u8)ch;mem[0xB8000+q*2+1]=7;if(++c.tcx>=TEXT_W){c.tcx=0;if(++c.tcy>=TEXT_H)c.tcy=TEXT_H-1;}}}
static void puts_dos(u16 s,u16 o,char end){for(u32 i=0;i<65535;i++){char ch=(char)mr8(s,(u16)(o+i));if(!ch||ch==end)break;outch(ch);}}
static void set_status(const char*s){kstrncpy(c.status,s,sizeof(c.status));}
