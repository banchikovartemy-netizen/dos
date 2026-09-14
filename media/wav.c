#include "media.h"
#include "sb16.h"
static u16 r16(const u8*p){return (u16)(p[0]|((u16)p[1]<<8));}static u32 r32(const u8*p){return (u32)p[0]|((u32)p[1]<<8)|((u32)p[2]<<16)|((u32)p[3]<<24);}static int eq4(const u8*p,const char*s){return p[0]==s[0]&&p[1]==s[1]&&p[2]==s[2]&&p[3]==s[3];}
static u8 conv[65000];
int wav_backend_ready(void){return sb16_ready();}
int wav_play(const u8*d,u32 sz){if(!d||sz<44||!eq4(d,"RIFF")||!eq4(d+8,"WAVE"))return 0;u16 fmt=0,ch=0,bits=0;u32 rate=0;const u8*pcm=0;u32 n=0;for(u32 p=12;p+8<=sz;){u32 len=r32(d+p+4);if(p+8+len>sz)break;if(eq4(d+p,"fmt ")&&len>=16){fmt=r16(d+p+8);ch=r16(d+p+10);rate=r32(d+p+12);bits=r16(d+p+22);}else if(eq4(d+p,"data")){pcm=d+p+8;n=len;}p+=8+((len+1)&~1u);}if(fmt!=1||!pcm||!n||!ch||(bits!=8&&bits!=16))return 0;u32 frames=n/((bits/8)*ch),out=0;if(frames>65000)frames=65000;for(u32 i=0;i<frames;i++){if(bits==8){u32 sum=0;for(u16 c=0;c<ch;c++)sum+=pcm[i*ch+c];conv[out++]=(u8)(sum/ch);}else{int sum=0;for(u16 c=0;c<ch;c++){u32 o=(i*ch+c)*2;sum+=(i16)(pcm[o]|((u16)pcm[o+1]<<8));}sum/=ch;conv[out++]=(u8)((sum>>8)+128);}}return sb16_play_u8(conv,out,rate);}
