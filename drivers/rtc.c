#include "rtc.h"
#include "io.h"
#include "lib.h"

static u8 cmos(u8 reg){outb(0x70,(u8)(reg|0x80));return inb(0x71);} 
static int updating(void){return cmos(0x0A)&0x80;}
static u8 bcd(u8 v){return (u8)((v&15)+((v>>4)*10));}
static void stable_read(u8 *s,u8*m,u8*h,u8*d,u8*mo,u8*y,u8*c,u8*rb){
    u8 s2,m2,h2,d2,mo2,y2,c2;
    do{
        while(updating());
        *s=cmos(0x00);*m=cmos(0x02);*h=cmos(0x04);*d=cmos(0x07);*mo=cmos(0x08);*y=cmos(0x09);*c=cmos(0x32);*rb=cmos(0x0B);
        while(updating());
        s2=cmos(0x00);m2=cmos(0x02);h2=cmos(0x04);d2=cmos(0x07);mo2=cmos(0x08);y2=cmos(0x09);c2=cmos(0x32);
    }while(*s!=s2||*m!=m2||*h!=h2||*d!=d2||*mo!=mo2||*y!=y2||*c!=c2);
}
void rtc_read(RtcTime*t){
    if(!t)return;u8 s,m,h,d,mo,y,c,rb;stable_read(&s,&m,&h,&d,&mo,&y,&c,&rb);
    int pm=h&0x80;h&=0x7F;
    if(!(rb&0x04)){s=bcd(s);m=bcd(m);h=bcd(h);d=bcd(d);mo=bcd(mo);y=bcd(y);c=bcd(c);}
    if(!(rb&0x02)){if(pm&&h<12)h=(u8)(h+12);if(!pm&&h==12)h=0;}
    t->second=s;t->minute=m;t->hour=h;t->day=d;t->month=mo;
    t->year=(u16)((c?c:20)*100u+y);
}
static void two(char*out,u8 v){out[0]=(char)('0'+(v/10)%10);out[1]=(char)('0'+v%10);}
const char*rtc_day_phase(void){static char p[6];RtcTime t;rtc_read(&t);kstrncpy(p,(t.hour>=6&&t.hour<18)?"DAY":"NIGHT",sizeof(p));return p;}
void rtc_format_hud(char*out,u32 n){
    if(!out||n<2)return;RtcTime t;rtc_read(&t);char b[40];
    b[0]=(char)('0'+(t.year/1000)%10);b[1]=(char)('0'+(t.year/100)%10);b[2]=(char)('0'+(t.year/10)%10);b[3]=(char)('0'+t.year%10);b[4]='-';two(b+5,t.month);b[7]='-';two(b+8,t.day);b[10]=' ';two(b+11,t.hour);b[13]=':';two(b+14,t.minute);b[16]=':';two(b+17,t.second);b[19]=' ';b[20]=0;
    kstrcat(b,(t.hour>=6&&t.hour<18)?"DAY":"NIGHT",sizeof(b));kstrncpy(out,b,n);
}
