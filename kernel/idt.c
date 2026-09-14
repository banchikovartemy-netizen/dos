#include "idt.h"
#include "io.h"

typedef struct __attribute__((packed)) { u16 off_lo, sel; u8 zero, flags; u16 off_hi; } IdtGate;
typedef struct __attribute__((packed)) { u16 limit; u32 base; } IdtPtr;
static IdtGate idt[256];

#define DECL(n) extern void isr##n(void)
DECL(0); DECL(1); DECL(2); DECL(3); DECL(4); DECL(5); DECL(6); DECL(7);
DECL(8); DECL(9); DECL(10); DECL(11); DECL(12); DECL(13); DECL(14); DECL(15);
DECL(16); DECL(17); DECL(18); DECL(19); DECL(20); DECL(21); DECL(22); DECL(23);
DECL(24); DECL(25); DECL(26); DECL(27); DECL(28); DECL(29); DECL(30); DECL(31);

static void set_gate(int n,void(*fn)(void),u16 cs){u32 a=(u32)fn;idt[n].off_lo=(u16)a;idt[n].sel=cs;idt[n].zero=0;idt[n].flags=0x8E;idt[n].off_hi=(u16)(a>>16);}
static void e9c(char c){outb(0xE9,(u8)c);}static void e9s(const char*s){while(*s)e9c(*s++);}
static void e9hex(u32 v){static const char h[]="0123456789ABCDEF";for(int i=7;i>=0;i--)e9c(h[(v>>(i*4))&15]);}
static void text(const char*s,int row,u8 attr){volatile u16*v=(volatile u16*)0xB8000;int x=0;while(*s&&x<80)v[row*80+x++]=(u16)(u8)*s++|((u16)attr<<8);}
static void hextext(u32 v,char*out){static const char h[]="0123456789ABCDEF";out[0]='0';out[1]='x';for(int i=0;i<8;i++)out[2+i]=h[(v>>(28-i*4))&15];out[10]=0;}

void idt_init(void){
    for(int i=0;i<256;i++){idt[i].off_lo=0;idt[i].sel=0;idt[i].zero=0;idt[i].flags=0;idt[i].off_hi=0;}
    u16 cs;__asm__ volatile("mov %%cs,%0":"=r"(cs));
    void(*f[32])(void)={isr0,isr1,isr2,isr3,isr4,isr5,isr6,isr7,isr8,isr9,isr10,isr11,isr12,isr13,isr14,isr15,isr16,isr17,isr18,isr19,isr20,isr21,isr22,isr23,isr24,isr25,isr26,isr27,isr28,isr29,isr30,isr31};
    for(int i=0;i<32;i++)set_gate(i,f[i],cs);
    IdtPtr p={(u16)(sizeof(idt)-1),(u32)idt};__asm__ volatile("lidt %0"::"m"(p));
    e9s("PCOS: IDT online\n");
}
__attribute__((noreturn)) void fault_handler(u32 vector,u32 error){
    char a[11],b[11];hextext(vector,a);hextext(error,b);
    e9s("PCOS FATAL EXCEPTION vector=");e9hex(vector);e9s(" error=");e9hex(error);e9s("\n");
    text("PCOS // FATAL CPU EXCEPTION",0,0x4F);
    text("VECTOR:",2,0x0F);text(a,3,0x0F);
    text("ERROR CODE:",5,0x0F);text(b,6,0x0F);
    text("SYSTEM HALTED - NO AUTOMATIC REBOOT",8,0x0F);
    for(;;){__asm__ volatile("cli; hlt");}
}
