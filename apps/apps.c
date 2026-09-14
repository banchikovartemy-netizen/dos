#include "apps.h"
#include "ui.h"
#include "vga.h"
#include "fs.h"
#include "lib.h"
#include "io.h"
#include "mouse.h"
#include "net.h"
#include "sb16.h"
#include "media.h"
#include "dos86.h"
#include "timer.h"
#include "multiboot.h"
#include "ata.h"
#include "legacy.h"

#define CX 20
#define LINE_W 56
#define LOG_BYTES 192
#define MAX_LINES 13

static u32 mem_kb=0;
static char cwd[96]="pc/files",term_in[96];static int term_len=0;
static char logbuf[MAX_LINES][LOG_BYTES];static int logcount=0;
static char note[4096];static int note_len=0;
static char calc_in[56],calc_result[64]="0";static int calc_len=0;
static char files_path[96]="pc";static int files_sel=0;
static char files_msg[192]="ARROWS/ENTER  BACKSPACE=UP  N FILE  M DIR  D DELETE";
static int player_track=0,player_sent=0;static int photo_sel=0;
static const char*track_names[]={"future.wav","machine.wav","night.wav"};
static int video_frame=0,video_frames=0,video_play=0;static u32 video_last=0;
static int game_sel=0;static char dos_out[512]="SELECT A GAME AND PRESS ENTER";static u32 dos_draw_last=0;
static char browser_url[128]="example.com";static int browser_len=11;
static char browser_page[1200]="PCOS BROWSER READY.\n\nSTART tools/web_bridge.py ON THE HOST, THEN TYPE A URL AND PRESS ENTER OR CLICK [GO].\n\nTHE BRIDGE USES REAL CHROMIUM FOR TLS, HTML, CSS AND JAVASCRIPT WHILE PCOS STAYS LIGHTWEIGHT.";

static u8 A(void){return(u8)(vga_fg()|(vga_bg()<<4));}
static u8 D(void){return(u8)(vga_dim()|(vga_bg()<<4));}
static u8 H(void){return(u8)(vga_hi()|(vga_bg()<<4));}

const char*apps_name(AppId a){
    static const char*n[]={"FILES","TERMINAL","NOTES","PLAYER","PHOTOS","VIDEO","CALCULATOR","GAMES","BROWSER","NETWORK","SYSTEM","SETTINGS"};
    return a<APP_COUNT?n[a]:"?";
}
static const char*type_name(FsType t){switch(t){case FS_DIR:return"DIR";case FS_FILE:return"FILE";case FS_APP:return"APP";case FS_DOS:return"DOS";case FS_IMAGE:return"IMAGE";case FS_AUDIO:return"AUDIO";case FS_VIDEO:return"VIDEO";default:return"?";}}
static void log_line(const char*s){if(logcount<MAX_LINES){kstrncpy(logbuf[logcount++],s,LOG_BYTES);return;}for(int i=1;i<MAX_LINES;i++)kstrncpy(logbuf[i-1],logbuf[i],LOG_BYTES);kstrncpy(logbuf[MAX_LINES-1],s,LOG_BYTES);}
static void log_num(const char*pre,u32 v){char l[LOG_BYTES],n[16];kstrncpy(l,pre,sizeof(l));kitoa((i32)v,n);kstrcat(l,n,sizeof(l));log_line(l);}
static void fullpath(char*out,usize n,const char*dir,const char*name){kstrncpy(out,dir,n);kstrcat(out,"/",n);kstrcat(out,name,n);}
static const u8*track(int i,u32*sz){char p[96];fullpath(p,sizeof(p),"pc/music",track_names[i]);return fs_read_path(p,sz);}
static void launch_doom(void){kstrncpy(dos_out,"DOOM 1: CHAINLOADING LEGACY DOS HDD2...",sizeof(dos_out));legacy_boot_drive(0x81);}
static void browser_open(void){browser_url[browser_len]=0;if(net_web_open(browser_url))kstrncpy(browser_page,"LOADING THROUGH PCOS WEB BRIDGE...",sizeof(browser_page));else kstrncpy(browser_page,"BROWSER COULD NOT SEND REQUEST. START FULL DRIVERS AND CHECK NETWORK.",sizeof(browser_page));}

static void terminal_backspace_utf8(void){if(term_len<=0)return;term_len--;while(term_len>0&&(((u8)term_in[term_len]&0xC0)==0x80))term_len--;term_in[term_len]=0;}
static void terminal_exec(void){
    term_in[term_len]=0;char q[LOG_BYTES]="$ ";kstrcat(q,term_in,sizeof(q));log_line(q);
    if(!term_len){}
    else if(!kstrcmp(term_in,"help")){
        log_line("COMMANDS: ls cd pwd cat echo touch mkdir rm mv write");
        log_line("mem net ping dhcp play dos doom legacydos browse theme clear");
        log_line("reboot shutdown // PCOS SHELL");
    }
    else if(!kstrcmp(term_in,"clear"))logcount=0;
    else if(!kstrcmp(term_in,"pwd"))log_line(cwd);
    else if(!kstrcmp(term_in,"ls")){
        int n=0;const FsEntry*e=fs_list(cwd,&n);char l[LOG_BYTES]="";
        for(int i=0;i<n;i++){if(kstrlen(l)+kstrlen(e[i].name)+2>=LINE_W){log_line(l);l[0]=0;}kstrcat(l,e[i].name,sizeof(l));kstrcat(l,"  ",sizeof(l));}
        if(l[0])log_line(l);if(!n)log_line("[ EMPTY ]");
    }
    else if(!kstrncmp(term_in,"cd ",3)){char p[96];if(fs_cd(cwd,term_in+3,p,sizeof(p)))kstrncpy(cwd,p,sizeof(cwd));else log_line("cd: directory not found");}
    else if(!kstrncmp(term_in,"cat ",4)){const char*s=fs_cat(cwd,term_in+4);log_line(s?s:"cat: file not found");}
    else if(!kstrncmp(term_in,"echo ",5))log_line(term_in+5);
    else if(!kstrncmp(term_in,"touch ",6))log_line(fs_touch(cwd,term_in+6)?"file created":"touch: error");
    else if(!kstrncmp(term_in,"mkdir ",6))log_line(fs_mkdir(cwd,term_in+6)?"directory created":"mkdir: error");
    else if(!kstrncmp(term_in,"rm ",3))log_line(fs_remove(cwd,term_in+3)?"deleted":"rm: error or directory not empty");
    else if(!kstrncmp(term_in,"mv ",3)){
        const char*s=term_in+3;char a[32],b[32];int i=0;while(*s&&*s!=' '&&i<31)a[i++]=*s++;a[i]=0;while(*s==' ')s++;kstrncpy(b,s,sizeof(b));
        log_line(fs_rename(cwd,a,b)?"renamed":"mv: error");
    }
    else if(!kstrncmp(term_in,"write ",6)){
        const char*s=term_in+6;char n[32],p[128];int i=0;while(*s&&*s!=' '&&i<31)n[i++]=*s++;n[i]=0;while(*s==' ')s++;fullpath(p,sizeof(p),cwd,n);
        log_line(fs_write_text(p,s)?"written":"write: overlay full");
    }
    else if(!kstrcmp(term_in,"mem"))log_num("RAM KB: ",mem_kb);
    else if(!kstrcmp(term_in,"net")){log_line(net_state());char x[18];net_get_ip(x);log_line(x);}
    else if(!kstrcmp(term_in,"ping")){net_ping_gateway();log_line("gateway ping queued");}
    else if(!kstrcmp(term_in,"dhcp")){net_request_dhcp();log_line("DHCP request sent");}
    else if(!kstrncmp(term_in,"play ",5)){
        int t=katoi(term_in+5);if(t<1)t=1;if(t>3)t=3;player_track=t-1;u32 z=0;const u8*d=track(player_track,&z);player_sent=d?wav_play(d,z):0;
        log_line(player_sent?"WAV sent to SB16":"audio unavailable or unsupported format");
    }
    else if(!kstrncmp(term_in,"dos ",4)){
        u32 z=0;const u8*d=fs_read("pc/games",term_in+4,&z);if(d&&dos86_run(d,z,term_in+4,dos_out,sizeof(dos_out)))log_line(dos_out);else log_line("dos: could not run file");
    }
    else if(!kstrcmp(term_in,"doom"))launch_doom();
    else if(!kstrcmp(term_in,"legacydos")){log_line("CHAINLOADING BIOS HDD2 / DL=81h...");legacy_boot_drive(0x81);}
    else if(!kstrncmp(term_in,"browse ",7)){kstrncpy(browser_url,term_in+7,sizeof(browser_url));browser_len=(int)kstrlen(browser_url);browser_open();log_line(net_web_state());}
    else if(!kstrncmp(term_in,"theme ",6)){int t=katoi(term_in+6);if(t>=1&&t<=5)vga_set_theme((u8)(t-1));}
    else if(!kstrcmp(term_in,"reboot"))outb(0x64,0xFE);
    else if(!kstrcmp(term_in,"shutdown")){outw(0x604,0x2000);outw(0x4004,0x3400);}
    else log_line("command not found");
    term_len=0;term_in[0]=0;
}

static void draw_terminal(void){
    ui_frame("TERMINAL // PCOS SHELL");
    vga_text(CX,4,"PCOS SHELL 0.7 // type help",D());
    for(int i=0;i<logcount;i++)vga_text_clip(CX,6+i,logbuf[i],LINE_W,A());
    char p[160];kstrncpy(p,cwd,sizeof(p));kstrcat(p,"$ ",sizeof(p));kstrcat(p,term_in,sizeof(p));vga_text_clip(CX,20,p,LINE_W,H());
    int x=CX+(int)kstrlen(p);if(x<78)vga_put(x,20,'_',H());
}
static void draw_files(void){
    ui_frame("FILES");vga_text_clip(CX,4,files_path,LINE_W,H());
    int n=0;const FsEntry*e=fs_list(files_path,&n);if(files_sel>=n)files_sel=n?n-1:0;
    for(int i=0;i<n&&i<13;i++){int y=6+i;vga_put(CX,y,i==files_sel?'>':' ',i==files_sel?H():A());vga_text_clip(CX+2,y,e[i].name,30,i==files_sel?H():A());vga_text(CX+35,y,type_name(e[i].type),D());}
    vga_text_clip(CX,21,files_msg,LINE_W,D());
}
static void draw_notes(void){
    ui_frame("NOTES // AUTOSAVE");int pos=0,y=4;
    while(pos<note_len&&y<22){char line[192];int j=0;while(pos<note_len&&note[pos]!='\n'&&j<(int)sizeof(line)-1)line[j++]=note[pos++];if(pos<note_len&&note[pos]=='\n')pos++;line[j]=0;vga_text_clip(CX,y++,line,55,A());}
    if(y<22)vga_text(CX,y,"_",H());vga_text(CX,23,"AUTOSAVE -> pc/files/notes.txt",D());
}
static char cover(u32 s,int x,int y){u32 v=s^(u32)(x*1103515245u)^(u32)(y*2654435761u);v^=v>>13;v*=1274126177u;static const char g[]=" .:+*#/@\\";return g[v%(sizeof(g)-1)];}
static void draw_player(void){
    ui_frame("PLAYER // ASCII COVER + WAV");const char*t=track_names[player_track];u32 seed=khash(t);
    vga_text(CX,4,"+--------------------+",D());for(int y=0;y<11;y++){vga_put(CX,5+y,'|',D());for(int x=0;x<20;x++)vga_put(CX+1+x,5+y,cover(seed,x,y),(x+y)%7?A():H());vga_put(CX+21,5+y,'|',D());}vga_text(CX,16,"+--------------------+",D());
    vga_text(CX+26,6,"TRACK",D());vga_text(CX+26,7,t,H());vga_text(CX+26,9,"AUDIO",D());vga_text(CX+26,10,wav_backend_ready()?"SB16 READY":"NO SB16",A());
    vga_text(CX+26,12,"SPACE / CLICK = PLAY",D());vga_text(CX+26,13,"LEFT / RIGHT = TRACK",D());vga_text(CX+26,15,player_sent?"STATUS::SENT":"STATUS::IDLE",H());
}
static void draw_photos(void){
    ui_frame("PHOTOS // BMP VIEWER");int n=0,idx[16],pn=0;const FsEntry*e=fs_list("pc/photos",&n);for(int i=0;i<n&&pn<16;i++)if(e[i].type==FS_IMAGE)idx[pn++]=i;if(photo_sel>=pn)photo_sel=pn?pn-1:0;
    if(!pn){vga_text(CX+14,10,"[ NO BMP FILES ]",H());return;}u32 z=0;const u8*d=fs_read("pc/photos",e[idx[photo_sel]].name,&z);
    if(!(d&&bmp_draw(d,z,CX+2,4,45,16))){vga_text(CX+12,9,"+--- ASCII PREVIEW ---+",H());vga_text(CX+15,11,"< BMP / HUD >",A());}
    char l[96]="< ";kstrcat(l,e[idx[photo_sel]].name,sizeof(l));kstrcat(l," >  LEFT/RIGHT",sizeof(l));vga_text_clip(CX,21,l,LINE_W,D());
}
static void draw_video(void){
    ui_frame("VIDEO // LIGHT AVI");u32 z=0;const u8*d=fs_read_path("pc/video/scan.avi",&z);if(d&&video_frames<=0)video_frames=avi_frame_count(d,z);if(video_frames>0&&video_frame>=video_frames)video_frame=0;
    if(!(d&&avi_draw_frame(d,z,video_frame,CX+2,4,45,15)))vga_text(CX+9,10,"[ GRAPHICS MODE REQUIRED ]",H());
    char n[16],l[48]="FRAME ";kitoa(video_frame+1,n);kstrcat(l,n,sizeof(l));kstrcat(l,"/",sizeof(l));kitoa(video_frames,n);kstrcat(l,n,sizeof(l));vga_text(CX,20,l,A());vga_text(CX,22,"SPACE PLAY/PAUSE // LEFT/RIGHT FRAME",D());
}

typedef struct{const char*s;int ok;}Parser;
static void skip(Parser*p){while(*p->s==' ')p->s++;}
static int expr(Parser*p);
static int factor(Parser*p){skip(p);int neg=0;if(*p->s=='-'){neg=1;p->s++;}skip(p);int v=0;if(*p->s=='('){p->s++;v=expr(p);skip(p);if(*p->s==')')p->s++;else p->ok=0;}else{if(!kisdigit(*p->s)){p->ok=0;return 0;}while(kisdigit(*p->s)){v=v*10+(*p->s-'0');p->s++;}}return neg?-v:v;}
static int term(Parser*p){int v=factor(p);for(;;){skip(p);char o=*p->s;if(o!='*'&&o!='/'&&o!='%')break;p->s++;int r=factor(p);if(o=='*')v*=r;else if(!r)p->ok=0;else if(o=='/')v/=r;else v%=r;}return v;}
static int expr(Parser*p){int v=term(p);for(;;){skip(p);char o=*p->s;if(o!='+'&&o!='-')break;p->s++;int r=term(p);v=o=='+'?v+r:v-r;}return v;}
static int evaluate(const char*s,int*ok){Parser p={s,1};int v=expr(&p);skip(&p);if(*p.s)p.ok=0;*ok=p.ok;return v;}
static void draw_calc(void){ui_frame("CALCULATOR // INTEGER");vga_text(CX,5,"EXPRESSION",D());vga_text_clip(CX,7,calc_in,50,H());vga_put(CX+calc_len,7,'_',H());vga_hline(CX,9,36,'-',D());vga_text(CX,11,"RESULT",D());vga_text(CX,13,calc_result,H());vga_text(CX,18,"OPERATORS: + - * / % ( )",A());}

static void draw_games(void){
    ui_frame(dos86_active()?"GAMES // DOS86 RUNNING":"GAMES // LIBRARY");
    if(dos86_active()){dos86_draw(CX,4,56,16);vga_hline(CX,20,56,'-',D());vga_text_clip(CX,21,dos86_status(),56,H());vga_text(CX,22,"ESC EXIT // KEYBOARD -> DOS // 1MB SANDBOX",D());return;}
    int n=0;const FsEntry*e=fs_list("pc/games",&n);int dosn=0;
    vga_put(CX,5,game_sel==0?'>':' ',game_sel==0?H():A());vga_text(CX+2,5,"DOOM 1 [LEGACY DOS HDD2]",game_sel==0?H():A());
    for(int i=0;i<n&&dosn<8;i++)if(e[i].type==FS_DOS){int y=6+dosn;vga_put(CX,y,(dosn+1)==game_sel?'>':' ',(dosn+1)==game_sel?H():A());vga_text(CX+2,y,e[i].name,(dosn+1)==game_sel?H():A());dosn++;}
    if(game_sel>dosn)game_sel=dosn;
    vga_text(CX,15,"ENTER / CLICK = RUN",D());vga_text(CX,16,"DOOM REQUIRES YOUR LEGAL DOS DISK + DOOM1.WAD",H());vga_text_clip(CX,18,dos86_level(),56,A());vga_text_clip(CX,20,dos_out,56,H());vga_text(CX,22,"DOS86=WINDOWED // DOOM=LEGACY CHAINLOAD",D());
}
static void browser_text(void){
    int pos=0,y=8;while(browser_page[pos]&&y<22){char line[64];int j=0;while(browser_page[pos]&&browser_page[pos]!='\n'&&j<55)line[j++]=browser_page[pos++];if(browser_page[pos]=='\n')pos++;line[j]=0;vga_text_clip(CX,y++,line,55,A());}
}
static void draw_browser(void){
    ui_frame("BROWSER // CHROMIUM WEB BRIDGE");vga_text(CX,4,"URL",D());vga_text(CX+4,4,"[",D());vga_text_clip(CX+5,4,browser_url,43,H());vga_text(CX+49,4,"]",D());vga_text(CX+51,4,"[GO]",H());
    vga_text(CX,6,net_web_state(),D());browser_text();vga_text(CX,23,"HOST BRIDGE 10.0.2.2:7777 // REAL CHROMIUM",D());
}
static void draw_network(void){
    ui_frame("NETWORK");char mac[18],ip[16],gw[16],dns[16],n[16];net_get_mac(mac);net_get_ip(ip);net_get_gateway(gw);net_get_dns(dns);
    ui_label(CX,4,"DRIVER",net_driver());ui_label(CX,6,"STATE",net_state());ui_label(CX,8,"MAC",mac);ui_label(CX,10,"IPv4",ip);ui_label(CX,12,"GATEWAY",gw);ui_label(CX,14,"DNS",dns);
    kitoa((i32)net_rx_packets(),n);ui_label(CX,16,"RX",n);kitoa((i32)net_tx_packets(),n);ui_label(CX,17,"TX",n);kitoa((i32)net_ping_replies(),n);ui_label(CX,18,"PING REPLY",n);vga_text(CX,21,"D = DHCP // P = PING GATEWAY",D());
}
static void vendor(char o[13]){u32 b,c,d;__asm__ volatile("cpuid":"=b"(b),"=d"(d),"=c"(c):"a"(0));kmemcpy(o,&b,4);kmemcpy(o+4,&d,4);kmemcpy(o+8,&c,4);o[12]=0;}
static void draw_system(void){
    ui_frame("SYSTEM // TELEMETRY");char v[13],n[20],disp[32]="";vendor(v);ui_label(CX,4,"CPU",v);kitoa((i32)mem_kb,n);ui_label(CX,6,"RAM KB",n);
    if(vga_framebuffer()){kitoa((i32)vga_px_width(),disp);kstrcat(disp,"x",sizeof(disp));kitoa((i32)vga_px_height(),n);kstrcat(disp,n,sizeof(disp));}else kstrncpy(disp,"VGA 80x25",sizeof(disp));
    ui_label(CX,8,"DISPLAY",disp);ui_label(CX,10,"INPUT","PS/2 KEYBOARD + MOUSE");ui_label(CX,12,"AUDIO",sb16_ready()?"SB16":"NONE");ui_label(CX,14,"STORAGE",fs_persistent()?"PCOSDATA / PERSIST":"RAM / TEMP");ui_label(CX,16,"NETWORK",net_driver());kitoa((i32)timer_ticks(),n);ui_label(CX,18,"PIT TICKS",n);ui_label(CX,20,"KERNEL","PCOS 0.7 i386");vga_text(CX,22,ata_ready()?ata_model():"NO IDE DATA DISK",D());
}
static void draw_settings(void){
    ui_frame("SETTINGS // HUD PROFILE");const char*t[]={"1 TERMINAL RED","2 TERMINAL GREEN","3 AMBER","4 ICE BLUE","5 MONOCHROME"};vga_text(CX,5,"THEME",D());for(int i=0;i<5;i++)vga_text(CX,7+i,t[i],vga_theme()==i?H():A());vga_text(CX,14,"VIDEO",D());vga_text(CX,16,vga_framebuffer()?"FRAMEBUFFER / AUTOSCALE":"EMERGENCY VGA TEXT",A());vga_text(CX,18,"F12 OR ALT+SHIFT = RU/EN INPUT",D());vga_text(CX,20,"NO BLUR / SHADOWS / WINDOW COMPOSITOR",D());
}

void apps_init(u32 magic,u32 mbi_addr){
    fs_init(magic,mbi_addr);if(magic==MULTIBOOT_BOOTLOADER_MAGIC){MultibootInfo*m=(MultibootInfo*)mbi_addr;if(m->flags&MBI_FLAG_MEM)mem_kb=m->mem_lower+m->mem_upper;}
    u32 n=0;const u8*d=fs_read_path("pc/files/notes.txt",&n);if(d){u32 z=n<sizeof(note)-1?n:sizeof(note)-1;kmemcpy(note,d,z);note[z]=0;note_len=(int)z;}
    log_line("PCOS SHELL READY");log_line("TYPE help FOR COMMANDS");
}
void apps_draw(AppId a){switch(a){case APP_FILES:draw_files();break;case APP_TERMINAL:draw_terminal();break;case APP_NOTES:draw_notes();break;case APP_PLAYER:draw_player();break;case APP_PHOTOS:draw_photos();break;case APP_VIDEO:draw_video();break;case APP_CALCULATOR:draw_calc();break;case APP_GAMES:draw_games();break;case APP_BROWSER:draw_browser();break;case APP_NETWORK:draw_network();break;case APP_SYSTEM:draw_system();break;case APP_SETTINGS:draw_settings();break;default:break;}}

static void note_backspace_utf8(void){if(note_len<=0)return;note_len--;while(note_len>0&&(((u8)note[note_len]&0xC0)==0x80))note_len--;}
static void files_open_selected(void){int n=0;const FsEntry*ls=fs_list(files_path,&n);if(!n)return;if(ls[files_sel].type==FS_DIR){char p[96];if(fs_cd(files_path,ls[files_sel].name,p,sizeof(p)))kstrncpy(files_path,p,sizeof(files_path));files_sel=0;}else{const char*c=fs_cat(files_path,ls[files_sel].name);kstrncpy(files_msg,c?c:"BINARY FILE // OPEN WITH THE CORRECT APP",sizeof(files_msg));}}
static void games_run_selected(void){
    if(game_sel==0){launch_doom();return;}int n=0,idx[16],dn=0;const FsEntry*ls=fs_list("pc/games",&n);for(int i=0;i<n&&dn<16;i++)if(ls[i].type==FS_DOS)idx[dn++]=i;int k=game_sel-1;if(k>=0&&k<dn){u32 z=0;const char*nm=ls[idx[k]].name;const u8*d=fs_read("pc/games",nm,&z);if(d&&dos86_start(d,z,nm))kstrncpy(dos_out,"PROGRAM STARTED",sizeof(dos_out));else kstrncpy(dos_out,"DOS86 LOAD ERROR",sizeof(dos_out));}}
void apps_key(AppId a,KeyEvent e){
    if(!e.pressed)return;
    if(a==APP_TERMINAL){if(e.special==KEY_ENTER)terminal_exec();else if(e.special==KEY_BACKSPACE&&term_len>0)terminal_backspace_utf8();else if(e.ch&&term_len<(int)sizeof(term_in)-1){term_in[term_len++]=e.ch;term_in[term_len]=0;}}
    else if(a==APP_FILES){int n=0;const FsEntry*ls=fs_list(files_path,&n);if(e.special==KEY_UP&&files_sel>0)files_sel--;else if(e.special==KEY_DOWN&&files_sel+1<n)files_sel++;else if(e.special==KEY_BACKSPACE){char p[96];if(fs_cd(files_path,"..",p,sizeof(p)))kstrncpy(files_path,p,sizeof(files_path));files_sel=0;}else if(e.special==KEY_ENTER&&n)files_open_selected();else if(e.ch=='n'||e.ch=='N'){fs_touch(files_path,"new.txt");kstrncpy(files_msg,"CREATED new.txt",sizeof(files_msg));}else if(e.ch=='m'||e.ch=='M'){fs_mkdir(files_path,"newdir");kstrncpy(files_msg,"CREATED newdir",sizeof(files_msg));}else if((e.ch=='d'||e.ch=='D')&&n)kstrncpy(files_msg,fs_remove(files_path,ls[files_sel].name)?"DELETED":"DELETE ERROR",sizeof(files_msg));}
    else if(a==APP_NOTES){if(e.special==KEY_ENTER&&note_len<(int)sizeof(note)-1)note[note_len++]='\n';else if(e.special==KEY_BACKSPACE&&note_len>0)note_backspace_utf8();else if(e.ch&&note_len<(int)sizeof(note)-1)note[note_len++]=e.ch;note[note_len]=0;fs_write_text("pc/files/notes.txt",note);}
    else if(a==APP_PLAYER){if(e.ch==' '){u32 z=0;const u8*d=track(player_track,&z);player_sent=d?wav_play(d,z):0;}else if(e.special==KEY_LEFT){player_track=(player_track+2)%3;player_sent=0;}else if(e.special==KEY_RIGHT){player_track=(player_track+1)%3;player_sent=0;}}
    else if(a==APP_PHOTOS){int n=0,pn=0;const FsEntry*e2=fs_list("pc/photos",&n);for(int i=0;i<n;i++)if(e2[i].type==FS_IMAGE)pn++;if(e.special==KEY_LEFT&&pn)photo_sel=(photo_sel+pn-1)%pn;else if(e.special==KEY_RIGHT&&pn)photo_sel=(photo_sel+1)%pn;}
    else if(a==APP_VIDEO){if(e.ch==' ')video_play=!video_play;else if(e.special==KEY_LEFT&&video_frames>0)video_frame=(video_frame+video_frames-1)%video_frames;else if(e.special==KEY_RIGHT&&video_frames>0)video_frame=(video_frame+1)%video_frames;}
    else if(a==APP_CALCULATOR){if(e.special==KEY_ENTER){int ok=0,r=evaluate(calc_in,&ok);if(ok)kitoa(r,calc_result);else kstrncpy(calc_result,"ERROR",sizeof(calc_result));}else if(e.special==KEY_BACKSPACE&&calc_len>0)calc_in[--calc_len]=0;else if(e.ch&&calc_len<(int)sizeof(calc_in)-1){calc_in[calc_len++]=e.ch;calc_in[calc_len]=0;}}
    else if(a==APP_GAMES){if(dos86_active()){if(e.special==KEY_ESC){dos86_stop();kstrncpy(dos_out,"PROGRAM STOPPED",sizeof(dos_out));}else dos86_key(e);}else{int n=0,dn=0;const FsEntry*ls=fs_list("pc/games",&n);for(int i=0;i<n;i++)if(ls[i].type==FS_DOS)dn++;if(e.ch=='l'||e.ch=='L'||e.ch=='d'||e.ch=='D')launch_doom();else if(e.special==KEY_UP&&game_sel>0)game_sel--;else if(e.special==KEY_DOWN&&game_sel<dn)game_sel++;else if(e.special==KEY_ENTER)games_run_selected();}}
    else if(a==APP_BROWSER){if(e.special==KEY_ENTER)browser_open();else if(e.special==KEY_BACKSPACE&&browser_len>0){browser_len--;while(browser_len>0&&(((u8)browser_url[browser_len]&0xC0)==0x80))browser_len--;browser_url[browser_len]=0;}else if(e.ch&&browser_len<(int)sizeof(browser_url)-1){browser_url[browser_len++]=e.ch;browser_url[browser_len]=0;}}
    else if(a==APP_NETWORK){if(e.ch=='d'||e.ch=='D')net_request_dhcp();else if(e.ch=='p'||e.ch=='P')net_ping_gateway();}
    else if(a==APP_SETTINGS&&e.ch>='1'&&e.ch<='5')vga_set_theme((u8)(e.ch-'1'));
}

void apps_mouse(AppId a,int x,int y,u8 buttons,u8 clicked){
    (void)buttons;if(!clicked)return;
    if(a==APP_FILES&&y>=6&&y<19){int n=0;fs_list(files_path,&n);int s=y-6;if(s<n){files_sel=s;files_open_selected();}}
    else if(a==APP_PLAYER){if(y>=11&&y<=15){u32 z=0;const u8*d=track(player_track,&z);player_sent=d?wav_play(d,z):0;}else if(x<CX+28)player_track=(player_track+2)%3;else player_track=(player_track+1)%3;}
    else if(a==APP_PHOTOS){int n=0,pn=0;const FsEntry*e=fs_list("pc/photos",&n);for(int i=0;i<n;i++)if(e[i].type==FS_IMAGE)pn++;if(pn)photo_sel=x<CX+28?(photo_sel+pn-1)%pn:(photo_sel+1)%pn;}
    else if(a==APP_VIDEO)video_play=!video_play;
    else if(a==APP_GAMES&&!dos86_active()&&y>=5&&y<=14){game_sel=y-5;games_run_selected();}
    else if(a==APP_BROWSER&&y==4&&x>=CX+50){browser_open();}
    else if(a==APP_NETWORK&&y>=20){if(x<CX+28)net_request_dhcp();else net_ping_gateway();}
    else if(a==APP_SETTINGS&&y>=7&&y<=11)vga_set_theme((u8)(y-7));
}

int apps_tick(AppId a){
    if(a==APP_GAMES&&dos86_active()){dos86_step(12000);u32 t=timer_ticks(),step=timer_hz()/20;if(step<1)step=1;if(t-dos_draw_last>=step){dos_draw_last=t;return 1;}}
    if(a==APP_VIDEO&&video_play&&video_frames>0){u32 t=timer_ticks(),step=timer_hz()/6;if(step<1)step=1;if(t-video_last>=step){video_last=t;video_frame=(video_frame+1)%video_frames;return 1;}}
    if(a==APP_BROWSER){char p[1200];if(net_web_read(p,sizeof(p))){kstrncpy(browser_page,p,sizeof(browser_page));return 1;}}
    return 0;
}
