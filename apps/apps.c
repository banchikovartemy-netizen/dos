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
#define MAX_LINES 13
static u32 mem_kb=0;
static char cwd[96]="pc/files",term_in[64];static int term_len=0;static char logbuf[MAX_LINES][LINE_W+1];static int logcount=0;
static char note[4096];static int note_len=0;
static char calc_in[56],calc_result[32]="0";static int calc_len=0;
static char files_path[96]="pc";static int files_sel=0;static char files_msg[58]="ARROWS/ENTER  BACKSPACE=UP  N FILE  M DIR  D DELETE";
static int player_track=0,player_sent=0;static int photo_sel=0;static const char*track_names[]={"future.wav","machine.wav","night.wav"};
static int video_frame=0,video_frames=0,video_play=0;static u32 video_last=0;
static int game_sel=0;static char dos_out[300]="SELECT COM/EXE AND PRESS ENTER";static u32 dos_draw_last=0;
static u8 A(void){return(u8)(vga_fg()|(vga_bg()<<4));}static u8 D(void){return(u8)(vga_dim()|(vga_bg()<<4));}static u8 H(void){return(u8)(vga_hi()|(vga_bg()<<4));}
const char*apps_name(AppId a){static const char*n[]={"FILES","TERMINAL","NOTES","PLAYER","PHOTOS","VIDEO","CALCULATOR","GAMES","NETWORK","SYSTEM","SETTINGS"};return a<APP_COUNT?n[a]:"?";}
static void log_line(const char*s){if(logcount<MAX_LINES){kstrncpy(logbuf[logcount++],s,LINE_W+1);return;}for(int i=1;i<MAX_LINES;i++)kstrncpy(logbuf[i-1],logbuf[i],LINE_W+1);kstrncpy(logbuf[MAX_LINES-1],s,LINE_W+1);}
static void log_num(const char*pre,u32 v){char l[LINE_W+1],n[16];kstrncpy(l,pre,sizeof(l));kitoa((i32)v,n);kstrcat(l,n,sizeof(l));log_line(l);}
static void fullpath(char*out,usize n,const char*dir,const char*name){kstrncpy(out,dir,n);kstrcat(out,"/",n);kstrcat(out,name,n);}
static const u8*track(int i,u32*sz){char p[96];fullpath(p,sizeof(p),"pc/music",track_names[i]);return fs_read_path(p,sz);}
static void terminal_exec(void){term_in[term_len]=0;char q[LINE_W+1]="$ ";kstrcat(q,term_in,sizeof(q));log_line(q);
 if(!term_len){}
 else if(!kstrcmp(term_in,"help")){log_line("ls cd pwd cat echo touch mkdir rm mv write mem net ping");log_line("dhcp play dos legacydos theme clear reboot shutdown");}
 else if(!kstrcmp(term_in,"clear"))logcount=0;
 else if(!kstrcmp(term_in,"pwd"))log_line(cwd);
 else if(!kstrcmp(term_in,"ls")){int n=0;const FsEntry*e=fs_list(cwd,&n);char l[LINE_W+1]="";for(int i=0;i<n;i++){if(kstrlen(l)+kstrlen(e[i].name)+2>=LINE_W){log_line(l);l[0]=0;}kstrcat(l,e[i].name,sizeof(l));kstrcat(l,"  ",sizeof(l));}if(l[0])log_line(l);if(!n)log_line("[EMPTY]");}
 else if(!kstrncmp(term_in,"cd ",3)){char p[96];if(fs_cd(cwd,term_in+3,p,sizeof(p)))kstrncpy(cwd,p,sizeof(cwd));else log_line("cd: directory not found");}
 else if(!kstrncmp(term_in,"cat ",4)){const char*s=fs_cat(cwd,term_in+4);log_line(s?s:"cat: file not found");}
 else if(!kstrncmp(term_in,"echo ",5))log_line(term_in+5);
 else if(!kstrncmp(term_in,"touch ",6))log_line(fs_touch(cwd,term_in+6)?"created":"touch: failed");
 else if(!kstrncmp(term_in,"mkdir ",6))log_line(fs_mkdir(cwd,term_in+6)?"directory created":"mkdir: failed");
 else if(!kstrncmp(term_in,"rm ",3))log_line(fs_remove(cwd,term_in+3)?"removed":"rm: failed/non-empty");
 else if(!kstrncmp(term_in,"mv ",3)){const char*s=term_in+3;char a[32],b[32];int i=0;while(*s&&*s!=' '&&i<31)a[i++]=*s++;a[i]=0;while(*s==' ')s++;kstrncpy(b,s,sizeof(b));log_line(fs_rename(cwd,a,b)?"renamed":"mv: failed");}
 else if(!kstrncmp(term_in,"write ",6)){const char*s=term_in+6;char n[32],p[128];int i=0;while(*s&&*s!=' '&&i<31)n[i++]=*s++;n[i]=0;while(*s==' ')s++;fullpath(p,sizeof(p),cwd,n);log_line(fs_write_text(p,s)?"written":"write: overlay full");}
 else if(!kstrcmp(term_in,"mem"))log_num("RAM KB: ",mem_kb);
 else if(!kstrcmp(term_in,"net")){log_line(net_state());char x[18];net_get_ip(x);log_line(x);}
 else if(!kstrcmp(term_in,"ping")){net_ping_gateway();log_line("gateway ping queued");}
 else if(!kstrcmp(term_in,"dhcp")){net_request_dhcp();log_line("DHCP discover sent");}
 else if(!kstrncmp(term_in,"play ",5)){int t=katoi(term_in+5);if(t<1)t=1;if(t>3)t=3;player_track=t-1;u32 z=0;const u8*d=track(player_track,&z);player_sent=d?wav_play(d,z):0;log_line(player_sent?"WAV sent to SB16":"audio unavailable/unsupported");}
 else if(!kstrncmp(term_in,"dos ",4)){u32 z=0;const u8*d=fs_read("pc/games",term_in+4,&z);if(d&&dos86_run(d,z,term_in+4,dos_out,sizeof(dos_out)))log_line(dos_out);else log_line("dos: cannot run file");}
 else if(!kstrcmp(term_in,"legacydos")){log_line("CHAINLOADING BIOS HDD2 / DL=81h...");legacy_boot_drive(0x81);}
 else if(!kstrncmp(term_in,"theme ",6)){int t=katoi(term_in+6);if(t>=1&&t<=5)vga_set_theme((u8)(t-1));}
 else if(!kstrcmp(term_in,"reboot"))outb(0x64,0xFE);
 else if(!kstrcmp(term_in,"shutdown")){outw(0x604,0x2000);outw(0x4004,0x3400);}
 else log_line("command not found");term_len=0;term_in[0]=0;}
static void draw_terminal(void){ui_frame("TERMINAL // LINUX-LIKE SHELL");vga_text(CX,4,"PCOS SHELL 0.3 // help",D());for(int i=0;i<logcount;i++)vga_text_clip(CX,6+i,logbuf[i],LINE_W,A());char p[128];kstrncpy(p,cwd,sizeof(p));kstrcat(p,"$ ",sizeof(p));kstrcat(p,term_in,sizeof(p));vga_text_clip(CX,20,p,LINE_W,H());int x=CX+(int)kstrlen(p);if(x<78)vga_put(x,20,'_',H());}
static void draw_files(void){ui_frame("FILES // PC TREE");vga_text_clip(CX,4,files_path,LINE_W,H());int n=0;const FsEntry*e=fs_list(files_path,&n);if(files_sel>=n)files_sel=n? n-1:0;for(int i=0;i<n&&i<13;i++){int y=6+i;vga_put(CX,y,i==files_sel?'>':' ',i==files_sel?H():A());vga_text_clip(CX+2,y,e[i].name,30,i==files_sel?H():A());vga_text(CX+35,y,fs_type_name(e[i].type),D());}vga_text_clip(CX,21,files_msg,LINE_W,D());}
static void draw_notes(void){ui_frame("NOTES // RAM OVERLAY");int y=4,col=0;for(int i=0;i<note_len&&y<22;i++){char c=note[i];if(c=='\n'||col>=55){y++;col=0;if(c=='\n')continue;}vga_put(CX+col,y,c,A());col++;}if(y<22)vga_put(CX+col,y,'_',H());vga_text(CX,23,"AUTO-SAVE -> pc/files/notes.txt",D());}
static char cover(u32 s,int x,int y){u32 v=s^(u32)(x*1103515245u)^(u32)(y*2654435761u);v^=v>>13;v*=1274126177u;static const char g[]=" .:+*#/@\\";return g[v%(sizeof(g)-1)];}
static void draw_player(void){ui_frame("PLAYER // ASCII COVER + WAV");const char*t=track_names[player_track];u32 seed=khash(t);vga_text(CX,4,t,H());for(int y=0;y<11;y++){vga_put(CX,y+6,'|',D());for(int x=0;x<24;x++)vga_put(CX+1+x,y+6,cover(seed,x,y),x==12||y==5?H():A());vga_put(CX+25,y+6,'|',D());}vga_text(CX+31,7,"[<<]  SPACE PLAY  [>>]",D());vga_text(CX+31,10,player_sent?"AUDIO :: DMA SENT":"AUDIO :: READY",A());vga_text(CX+31,12,sb16_ready()?"DEVICE :: SB16":"DEVICE :: NONE",D());vga_text(CX,20,"UP/DOWN TRACK  SPACE PLAY",D());}
static void draw_photos(void){ui_frame("PHOTOS // BMP BROWSER");int n=0;const FsEntry*e=fs_list("pc/photos",&n);if(!n){vga_text(CX,5,"NO IMAGES",D());return;}if(photo_sel>=n)photo_sel=n-1;vga_text(CX,4,e[photo_sel].name,H());u32 z=0;const u8*d=fs_read("pc/photos",e[photo_sel].name,&z);if(vga_framebuffer()&&d&&bmp_draw(d,z,250,150,800,500))vga_text(CX,21,"BITMAP RENDERED IN FRAMEBUFFER // ARROWS",D());else{vga_text(CX,7,"BMP 24-BIT IMAGE",A());vga_text(CX,9,"FRAMEBUFFER REQUIRED FOR PIXEL PREVIEW",D());} }
static void draw_video(void){ui_frame("VIDEO // UNCOMPRESSED AVI");u32 z=0;const u8*d=fs_read_path("pc/video/scan.avi",&z);video_frames=d?avi_frames(d,z):0;if(!video_frames){vga_text(CX,5,"NO/UNSUPPORTED AVI",D());return;}char n[16];kitoa(video_frame,n);vga_text(CX,4,video_play?"PLAYING":"PAUSED",video_play?H():D());vga_text(CX+14,4,"FRAME",D());vga_text(CX+20,4,n,A());kitoa(video_frames,n);vga_text(CX+25,4,"/",D());vga_text(CX+27,4,n,A());if(vga_framebuffer())avi_draw_frame(d,z,video_frame,250,180,800,500);vga_text(CX,21,"SPACE PLAY/PAUSE   LEFT/RIGHT FRAME",D());}
static int parse_expr(const char**s,int*ok);static void skip(const char**s){while(**s==' ')(*s)++;}static int factor(const char**s,int*ok){skip(s);int sign=1;if(**s=='-'){sign=-1;(*s)++;}skip(s);if(**s=='('){(*s)++;int v=parse_expr(s,ok);skip(s);if(**s!=')'){*ok=0;return 0;}(*s)++;return sign*v;}if(!kisdigit(**s)){*ok=0;return 0;}int v=0;while(kisdigit(**s)){v=v*10+(**s-'0');(*s)++;}return sign*v;}static int term(const char**s,int*ok){int v=factor(s,ok);for(;;){skip(s);char op=**s;if(op!='*'&&op!='/')break;(*s)++;int b=factor(s,ok);if(!*ok)return 0;if(op=='*')v*=b;else{if(!b){*ok=0;return 0;}v/=b;}}return v;}static int parse_expr(const char**s,int*ok){int v=term(s,ok);for(;;){skip(s);char op=**s;if(op!='+'&&op!='-')break;(*s)++;int b=term(s,ok);if(!*ok)return 0;v=op=='+'?v+b:v-b;}return v;}
static void calc_eval(void){calc_in[calc_len]=0;const char*s=calc_in;int ok=1,v=parse_expr(&s,&ok);skip(&s);if(*s)ok=0;if(ok)kitoa(v,calc_result);else kstrncpy(calc_result,"ERROR",sizeof(calc_result));}
static void draw_calc(void){ui_frame("CALCULATOR // INTEGER CORE");vga_text(CX,6,"EXPRESSION",D());vga_text_clip(CX,8,calc_in,54,H());vga_text(CX,11,"RESULT",D());vga_text(CX,13,calc_result,A());vga_text(CX,18,"SUPPORTED: + - * / ( )",D());vga_text(CX,20,"ENTER CALCULATE   C CLEAR",D());}
static void draw_games(void){ui_frame("GAMES // DOS86 + LEGACY DOS");if(dos86_active()){vga_text(CX,4,"[ DOS86 RUNNING ]   ESC EXIT",H());if(dos86_video_active()){dos86_video_draw(250,150,800,500);vga_text(CX,21,"VGA 320x200 / MODE13-MODEX VIRTUAL DISPLAY",D());}else vga_text_clip(CX,6,dos86_output(),LINE_W,A());return;}int n=0;const FsEntry*e=fs_list("pc/games",&n);if(game_sel>=n)game_sel=n?n-1:0;for(int i=0;i<n&&i<9;i++){vga_put(CX,5+i,i==game_sel?'>':' ',i==game_sel?H():A());vga_text(CX+2,5+i,e[i].name,i==game_sel?H():A());vga_text(CX+34,5+i,fs_type_name(e[i].type),D());}vga_text_clip(CX,16,dos_out,LINE_W,A());vga_text(CX,20,"ENTER: DOS86 WINDOWED COM/MZ EXE",D());vga_text(CX,21,"L: LEGACY DOS HDD2 (386/DOS4GW NATIVE)",H());vga_text(CX,22,"LEGACY MODE REBOOTS/CHAINLOADS BIOS DRIVE 81h",D());}
static void draw_network(void){ui_frame("NETWORK // INTERNET MANAGER");char s[32];ui_label(CX,4,"DRIVER",net_driver());ui_label(CX,6,"STATE",net_state());net_get_mac(s);ui_label(CX,8,"MAC",s);net_get_ip(s);ui_label(CX,10,"IP",s);net_get_gateway(s);ui_label(CX,12,"GATEWAY",s);net_get_dns(s);ui_label(CX,14,"DNS",s);char n[16];kitoa((i32)net_rx_packets(),n);ui_label(CX,16,"RX PACKETS",n);kitoa((i32)net_tx_packets(),n);ui_label(CX,18,"TX PACKETS",n);vga_text(CX,21,"D DHCP       P PING GATEWAY",D());vga_text(CX,22,net_last_message(),A());}
static void vendor(char o[13]){u32 b=0,c=0,d=0;__asm__ volatile("cpuid":"=b"(b),"=c"(c),"=d"(d):"a"(0));*(u32*)(o+0)=b;*(u32*)(o+4)=d;*(u32*)(o+8)=c;o[12]=0;}
static void draw_system(void){ui_frame("SYSTEM // TELEMETRY");char v[13],n[20],disp[32]="";vendor(v);ui_label(CX,4,"CPU",v);kitoa((i32)mem_kb,n);ui_label(CX,6,"RAM KB",n);if(vga_framebuffer()){kitoa((i32)vga_px_width(),disp);kstrcat(disp,"x",sizeof(disp));kitoa((i32)vga_px_height(),n);kstrcat(disp,n,sizeof(disp));}else kstrncpy(disp,"VGA 80x25",sizeof(disp));ui_label(CX,8,"DISPLAY",disp);ui_label(CX,10,"INPUT","PS/2 KB + MOUSE");ui_label(CX,12,"AUDIO",sb16_ready()?"SB16":"NONE");ui_label(CX,14,"STORAGE",fs_persistent()?"PCOSDATA / PERSIST":"RAM OVERLAY");ui_label(CX,16,"NETWORK",net_driver());kitoa((i32)timer_ticks(),n);ui_label(CX,18,"PIT TICKS",n);ui_label(CX,20,"KERNEL","PCOS 0.3 i686");vga_text(CX,22,ata_ready()?ata_model():"NO IDE DATA DEVICE",D());}
static void draw_settings(void){ui_frame("SETTINGS // LOW POWER DISPLAY");vga_text(CX,5,"THEME",D());vga_text(CX,7,"1 TERMINAL RED",A());vga_text(CX,8,"2 TERMINAL GREEN",A());vga_text(CX,9,"3 AMBER",A());vga_text(CX,10,"4 ICE BLUE",A());vga_text(CX,11,"5 MONOCHROME",A());vga_text(CX,14,"DISPLAY MODE IS SELECTED BY GRUB",D());vga_text(CX,15,"800x600 / 1024x768 / 720p / 1080p / 1440p",A());vga_text(CX,18,"HUD: INTEGER SCALE / NO BLUR / NO COMPOSITOR",D());vga_text(CX,20,"PRESS 1..5 TO CHANGE THEME",D());}
void apps_draw(AppId a){switch(a){case APP_FILES:draw_files();break;case APP_TERMINAL:draw_terminal();break;case APP_NOTES:draw_notes();break;case APP_PLAYER:draw_player();break;case APP_PHOTOS:draw_photos();break;case APP_VIDEO:draw_video();break;case APP_CALCULATOR:draw_calc();break;case APP_GAMES:draw_games();break;case APP_NETWORK:draw_network();break;case APP_SYSTEM:draw_system();break;case APP_SETTINGS:draw_settings();break;default:break;}}
void apps_init(u32 magic,u32 mbi){if(magic==MULTIBOOT_BOOTLOADER_MAGIC){MultibootInfo*m=(MultibootInfo*)mbi;if(m->flags&1)mem_kb=m->mem_lower+m->mem_upper;}fs_init(magic,mbi);u32 z=0;const u8*d=fs_read_path("pc/files/notes.txt",&z);if(d){note_len=z<sizeof(note)-1?(int)z:(int)sizeof(note)-1;kmemcpy(note,d,(usize)note_len);note[note_len]=0;}log_line("PCOS 0.3 ONLINE");log_line("TYPE help");}
static void files_key(KeyEvent e){int n=0;const FsEntry*l=fs_list(files_path,&n);if(e.special==KEY_UP&&files_sel>0)files_sel--;else if(e.special==KEY_DOWN&&files_sel+1<n)files_sel++;else if(e.special==KEY_BACKSPACE){char p[96];if(fs_cd(files_path,"..",p,sizeof(p)))kstrncpy(files_path,p,sizeof(files_path));files_sel=0;}else if(e.special==KEY_ENTER&&n){if(l[files_sel].type==FS_DIR){char p[96];if(fs_cd(files_path,l[files_sel].name,p,sizeof(p)))kstrncpy(files_path,p,sizeof(files_path));files_sel=0;}else{kstrncpy(files_msg,l[files_sel].name,sizeof(files_msg));}}else if((e.ch=='n'||e.ch=='N')){char nm[24]="new.txt";fs_touch(files_path,nm);kstrncpy(files_msg,"CREATED new.txt",sizeof(files_msg));}else if(e.ch=='m'||e.ch=='M'){fs_mkdir(files_path,"newdir");kstrncpy(files_msg,"CREATED newdir",sizeof(files_msg));}else if((e.ch=='d'||e.ch=='D')&&n){char nm[48];kstrncpy(nm,l[files_sel].name,sizeof(nm));kstrncpy(files_msg,fs_remove(files_path,nm)?"REMOVED":"REMOVE FAILED / NONEMPTY",sizeof(files_msg));}}
void apps_key(AppId a,KeyEvent e){if(a==APP_TERMINAL){if(e.special==KEY_ENTER)terminal_exec();else if(e.special==KEY_BACKSPACE){if(term_len)term_len--;}else if(e.ch>=32&&e.ch<127&&term_len<(int)sizeof(term_in)-1)term_in[term_len++]=e.ch;}
 else if(a==APP_FILES)files_key(e);
 else if(a==APP_NOTES){if(e.special==KEY_BACKSPACE){if(note_len)note_len--;}else if(e.special==KEY_ENTER&&note_len<(int)sizeof(note)-1)note[note_len++]='\n';else if(e.ch>=32&&e.ch<127&&note_len<(int)sizeof(note)-1)note[note_len++]=e.ch;note[note_len]=0;fs_write_text("pc/files/notes.txt",note);}
 else if(a==APP_CALCULATOR){if(e.special==KEY_ENTER)calc_eval();else if(e.special==KEY_BACKSPACE){if(calc_len)calc_len--;}else if(e.ch=='c'||e.ch=='C'){calc_len=0;calc_in[0]=0;kstrncpy(calc_result,"0",sizeof(calc_result));}else if((kisdigit(e.ch)||e.ch=='+'||e.ch=='-'||e.ch=='*'||e.ch=='/'||e.ch=='('||e.ch==')'||e.ch==' ')&&calc_len<(int)sizeof(calc_in)-1)calc_in[calc_len++]=e.ch;calc_in[calc_len]=0;}
 else if(a==APP_PLAYER){if(e.special==KEY_UP){if(player_track>0)player_track--;}else if(e.special==KEY_DOWN){if(player_track<2)player_track++;}else if(e.ch==' '){u32 z=0;const u8*d=track(player_track,&z);player_sent=d?wav_play(d,z):0;}}
 else if(a==APP_PHOTOS){int n=0;fs_list("pc/photos",&n);if(e.special==KEY_LEFT||e.special==KEY_UP){if(photo_sel>0)photo_sel--;}else if(e.special==KEY_RIGHT||e.special==KEY_DOWN){if(photo_sel+1<n)photo_sel++;}}
 else if(a==APP_VIDEO){if(e.ch==' '){video_play=!video_play;video_last=timer_ticks();}else if(e.special==KEY_LEFT&&video_frame>0)video_frame--;else if(e.special==KEY_RIGHT&&video_frame+1<video_frames)video_frame++;}
 else if(a==APP_GAMES){if(dos86_active()){if(e.special==KEY_ESC)dos86_stop();else dos86_key(e);return;}int n=0;const FsEntry*l=fs_list("pc/games",&n);if(e.special==KEY_UP&&game_sel>0)game_sel--;else if(e.special==KEY_DOWN&&game_sel+1<n)game_sel++;else if(e.special==KEY_ENTER&&n&&(l[game_sel].type==FS_DOS||l[game_sel].type==FS_FILE)){u32 z=0;const u8*d=fs_read("pc/games",l[game_sel].name,&z);if(d&&dos86_start(d,z,l[game_sel].name)){kstrncpy(dos_out,"DOS86 STARTED // ESC TO STOP",sizeof(dos_out));dos_draw_last=timer_ticks();}else kstrncpy(dos_out,"DOS86: UNSUPPORTED/FAILED",sizeof(dos_out));}else if(e.ch=='l'||e.ch=='L'){kstrncpy(dos_out,"CHAINLOADING BIOS HDD2...",sizeof(dos_out));legacy_boot_drive(0x81);}}
 else if(a==APP_NETWORK){if(e.ch=='d'||e.ch=='D')net_request_dhcp();else if(e.ch=='p'||e.ch=='P')net_ping_gateway();}
 else if(a==APP_SETTINGS&&e.ch>='1'&&e.ch<='5')vga_set_theme((u8)(e.ch-'1'));}
int apps_tick(AppId a){if(a==APP_GAMES&&dos86_active()){dos86_step(18000);u32 now=timer_ticks();if(now-dos_draw_last>=3){dos_draw_last=now;return 1;}if(!dos86_active()){kstrncpy(dos_out,dos86_output(),sizeof(dos_out));return 1;}}if(a==APP_VIDEO&&video_play&&video_frames>0){u32 now=timer_ticks();if(now-video_last>=16){video_last=now;video_frame=(video_frame+1)%video_frames;return 1;}}return 0;}
