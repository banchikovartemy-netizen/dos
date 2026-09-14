#include "fs.h"
#include "lib.h"
#include "multiboot.h"
#include "ata.h"
#define MAX_TAR 128
#define MAX_OVERLAY 20
#define RAM_FILE_SIZE 4096
#define OV_WHITEOUT 255
#define DATA_MAGIC_LBA 0u
#define DATA_HDR_LBA 1u
#define DATA_REC_LBA 2u
#define DATA_REC_SECTORS 9u
#define DATA_REC_BYTES (DATA_REC_SECTORS*512u)

typedef struct{const char *name;const u8 *data;u32 size;}TarFile;
typedef struct{int used;u8 type;char path[96];u8 data[RAM_FILE_SIZE];u32 size;}Overlay;
static TarFile tarf[MAX_TAR];static int tarc=0;static Overlay ov[MAX_OVERLAY];static FsEntry listbuf[48];static char catbuf[1024];
static int persist_on=0;static u8 diskrec[DATA_REC_BYTES] __attribute__((aligned(4)));
static u32 octal(const char*s,int n){u32 v=0;for(int i=0;i<n&&s[i];i++)if(s[i]>='0'&&s[i]<='7')v=(v<<3)+(u32)(s[i]-'0');return v;}
static int zero512(const u8*p){for(int i=0;i<512;i++)if(p[i])return 0;return 1;}
static FsType type_for_name(const char*n){usize l=kstrlen(n);if(l>=4&&!kstrcmp(n+l-4,".app"))return FS_APP;if(l>=4&&(!kstrcmp(n+l-4,".com")||!kstrcmp(n+l-4,".exe")||!kstrcmp(n+l-4,".COM")||!kstrcmp(n+l-4,".EXE")))return FS_DOS;if(l>=4&&!kstrcmp(n+l-4,".bmp"))return FS_IMAGE;if(l>=4&&!kstrcmp(n+l-4,".wav"))return FS_AUDIO;if(l>=4&&!kstrcmp(n+l-4,".avi"))return FS_VIDEO;return FS_FILE;}
static void join(char*out,usize n,const char*a,const char*b){kstrncpy(out,a,n);usize l=kstrlen(out);if(l&&l+1<n&&out[l-1]!='/'){out[l++]='/';out[l]=0;}kstrncpy(out+l,b,n-l);}
static int valid_name(const char*n){if(!n||!*n||!kstrcmp(n,".")||!kstrcmp(n,".."))return 0;for(;*n;n++)if(*n=='/'||*n=='\\')return 0;return 1;}
static Overlay*ov_find(const char*path){for(int i=0;i<MAX_OVERLAY;i++)if(ov[i].used&&!kstrcmp(ov[i].path,path))return &ov[i];return 0;}
static Overlay*ov_alloc(const char*path,u8 type){Overlay*e=ov_find(path);if(e){e->type=type;e->size=0;return e;}for(int i=0;i<MAX_OVERLAY;i++)if(!ov[i].used){ov[i].used=1;ov[i].type=type;ov[i].size=0;kstrncpy(ov[i].path,path,sizeof(ov[i].path));return &ov[i];}return 0;}
static int ov_index(Overlay*e){return e?(int)(e-ov):-1;}
static int disk_readn(u32 lba,u8*dst,u32 sectors){for(u32 i=0;i<sectors;i++)if(!ata_read_sector(lba+i,dst+i*512u))return 0;return 1;}
static int disk_writen(u32 lba,const u8*src,u32 sectors){for(u32 i=0;i<sectors;i++)if(!ata_write_sector(lba+i,src+i*512u))return 0;return 1;}
static int magic_eq(const u8*p,const char*s,int n){for(int i=0;i<n;i++)if(p[i]!=(u8)s[i])return 0;return 1;}
static void persist_slot(int i){if(!persist_on||i<0||i>=MAX_OVERLAY)return;kmemset(diskrec,0,sizeof(diskrec));Overlay*e=&ov[i];if(e->used){diskrec[0]=1;diskrec[1]=e->type;diskrec[4]=(u8)e->size;diskrec[5]=(u8)(e->size>>8);diskrec[6]=(u8)(e->size>>16);diskrec[7]=(u8)(e->size>>24);kstrncpy((char*)diskrec+8,e->path,96);if(e->size&&e->type!=FS_DIR&&e->type!=OV_WHITEOUT)kmemcpy(diskrec+104,e->data,e->size>RAM_FILE_SIZE?RAM_FILE_SIZE:e->size);}disk_writen(DATA_REC_LBA+(u32)i*DATA_REC_SECTORS,diskrec,DATA_REC_SECTORS);}
static void persist_load(void){persist_on=0;if(!ata_ready())return;u8 sec[512] __attribute__((aligned(4)));if(!ata_read_sector(DATA_MAGIC_LBA,sec)||!magic_eq(sec,"PCOSDATA",8))return;if(!ata_read_sector(DATA_HDR_LBA,sec)||!magic_eq(sec,"PCOSOV2",7))return;persist_on=1;for(int i=0;i<MAX_OVERLAY;i++){if(!disk_readn(DATA_REC_LBA+(u32)i*DATA_REC_SECTORS,diskrec,DATA_REC_SECTORS))break;if(!diskrec[0])continue;u8 type=diskrec[1];u32 sz=(u32)diskrec[4]|((u32)diskrec[5]<<8)|((u32)diskrec[6]<<16)|((u32)diskrec[7]<<24);if(sz>RAM_FILE_SIZE)sz=RAM_FILE_SIZE;ov[i].used=1;ov[i].type=type;ov[i].size=sz;kstrncpy(ov[i].path,(const char*)diskrec+8,sizeof(ov[i].path));if(type!=FS_DIR&&type!=OV_WHITEOUT&&sz)kmemcpy(ov[i].data,diskrec+104,sz);if(sz<RAM_FILE_SIZE)ov[i].data[sz]=0;}}
static int tar_find(const char*path,const u8**data,u32*size){for(int i=0;i<tarc;i++)if(!kstrcmp(tarf[i].name,path)){if(data)*data=tarf[i].data;if(size)*size=tarf[i].size;return 1;}return 0;}
void fs_init(u32 magic,u32 mbi_addr){tarc=0;persist_on=0;for(int i=0;i<MAX_OVERLAY;i++)ov[i].used=0;if(magic==MULTIBOOT_BOOTLOADER_MAGIC){MultibootInfo*m=(MultibootInfo*)mbi_addr;if((m->flags&MBI_FLAG_MODS)&&m->mods_count){MultibootModule*mods=(MultibootModule*)m->mods_addr;const u8*p=(const u8*)mods[0].mod_start,*end=(const u8*)mods[0].mod_end;while(p+512<=end&&!zero512(p)&&tarc<MAX_TAR){const char*name=(const char*)p;u32 sz=octal((const char*)p+124,12);char type=(char)p[156];if(name[0]&&type!='5'){tarf[tarc].name=name;tarf[tarc].data=p+512;tarf[tarc].size=sz;tarc++;}p+=512+((sz+511)&~511u);}}}persist_load();}
const u8*fs_read_path(const char*path,u32*size){Overlay*e=ov_find(path);if(e){if(e->type==OV_WHITEOUT||e->type==FS_DIR){if(size)*size=0;return 0;}if(size)*size=e->size;return e->data;}const u8*d=0;u32 n=0;if(tar_find(path,&d,&n)){if(size)*size=n;return d;}if(size)*size=0;return 0;}
const u8*fs_read(const char*cwd,const char*name,u32*size){char p[128];join(p,sizeof(p),cwd,name);return fs_read_path(p,size);}
static int child_name(const char*full,const char*parent,const char**name,int*len){usize pl=kstrlen(parent);if(kstrncmp(full,parent,pl)||full[pl]!='/')return 0;const char*r=full+pl+1;if(!*r)return 0;const char*s=kstrchr(r,'/');*name=r;*len=s?(int)(s-r):(int)kstrlen(r);return 1;}
static int entry_index(const char*n,int len){for(int i=0;i<48;i++)if(listbuf[i].name[0]&&(int)kstrlen(listbuf[i].name)==len&&!kstrncmp(listbuf[i].name,n,(usize)len))return i;return-1;}
static int add_entry(const char*n,int len,FsType t,u32 size){int ex=entry_index(n,len);if(ex>=0){listbuf[ex].type=t;listbuf[ex].size=size;return 0;}if(len<=0||len>=48)return 0;for(int i=0;i<48;i++)if(!listbuf[i].name[0]){for(int j=0;j<len;j++)listbuf[i].name[j]=n[j];listbuf[i].name[len]=0;listbuf[i].type=t;listbuf[i].size=size;return 1;}return 0;}
static int child_hidden(const char*parent,const char*n,int len){char p[128],tmp[48];for(int i=0;i<len&&i<47;i++)tmp[i]=n[i];tmp[len<47?len:47]=0;join(p,sizeof(p),parent,tmp);Overlay*e=ov_find(p);return e&&e->type==OV_WHITEOUT;}
const FsEntry*fs_list(const char*path,int*count){for(int i=0;i<48;i++)listbuf[i].name[0]=0;int n=0;for(int i=0;i<tarc;i++){const char*r;int len;if(!child_name(tarf[i].name,path,&r,&len)||child_hidden(path,r,len))continue;const char*s=kstrchr(r,'/');FsType t=s?FS_DIR:type_for_name(r);if(add_entry(r,len,t,s?0:tarf[i].size))n++;}for(int i=0;i<MAX_OVERLAY;i++)if(ov[i].used&&ov[i].type!=OV_WHITEOUT){const char*r;int len;if(!child_name(ov[i].path,path,&r,&len))continue;const char*s=kstrchr(r,'/');FsType t=s?FS_DIR:(ov[i].type==FS_DIR?FS_DIR:type_for_name(r));if(add_entry(r,len,t,s?0:ov[i].size))n++;}*count=n;return listbuf;}
const char*fs_cat(const char*path,const char*name){u32 sz=0;const u8*d=fs_read(path,name,&sz);if(!d)return 0;u32 n=sz<sizeof(catbuf)-1?sz:sizeof(catbuf)-1;for(u32 i=0;i<n;i++){u8 c=d[i];catbuf[i]=(c==0?' ':((c>=32||c=='\n'||c=='\r'||c=='\t')?(char)c:'.'));}catbuf[n]=0;return catbuf;}
int fs_exists(const char*path){Overlay*e=ov_find(path);if(e)return e->type!=OV_WHITEOUT;const u8*d;u32 s;if(tar_find(path,&d,&s))return 1;/* directory can be implicit */int n=0;fs_list(path,&n);return n>0;}
int fs_cd(const char*cwd,const char*target,char*out,usize outn){if(!kstrcmp(target,"/")||!kstrcmp(target,"pc")){kstrncpy(out,"pc",outn);return 1;}if(!kstrcmp(target,"~")){kstrncpy(out,"pc/files",outn);return 1;}if(!kstrcmp(target,"..")){if(!kstrcmp(cwd,"pc")){kstrncpy(out,"pc",outn);return 1;}int last=-1;for(int i=0;cwd[i];i++)if(cwd[i]=='/')last=i;if(last<0)kstrncpy(out,"pc",outn);else{usize i=0;for(;i<(usize)last&&i+1<outn;i++)out[i]=cwd[i];out[i]=0;}return 1;}int n=0;const FsEntry*e=fs_list(cwd,&n);for(int i=0;i<n;i++)if(e[i].type==FS_DIR&&!kstrcmp(e[i].name,target)){join(out,outn,cwd,target);return 1;}return 0;}
int fs_write_text(const char*path,const char*text){Overlay*e=ov_alloc(path,FS_FILE);if(!e)return 0;u32 n=(u32)kstrlen(text);if(n>=RAM_FILE_SIZE)n=RAM_FILE_SIZE-1;kmemcpy(e->data,text,n);e->data[n]=0;e->size=n;persist_slot(ov_index(e));return 1;}
int fs_touch(const char*cwd,const char*name){if(!valid_name(name))return 0;char p[128];join(p,sizeof(p),cwd,name);if(fs_exists(p))return 1;return fs_write_text(p,"");}
int fs_mkdir(const char*cwd,const char*name){if(!valid_name(name))return 0;char p[128];join(p,sizeof(p),cwd,name);if(fs_exists(p))return 0;Overlay*e=ov_alloc(p,FS_DIR);if(!e)return 0;persist_slot(ov_index(e));return 1;}
int fs_remove(const char*cwd,const char*name){if(!valid_name(name))return 0;char p[128];join(p,sizeof(p),cwd,name);int n=0;fs_list(p,&n);if(n>0)return 0;Overlay*e=ov_find(p);if(e&&e->type!=OV_WHITEOUT){e->type=OV_WHITEOUT;e->size=0;persist_slot(ov_index(e));return 1;}const u8*d;u32 s;if(tar_find(p,&d,&s)){e=ov_alloc(p,OV_WHITEOUT);if(!e)return 0;persist_slot(ov_index(e));return 1;}/* allow removing implicit empty overlay dir */return 0;}
int fs_rename(const char*cwd,const char*oldname,const char*newname){if(!valid_name(oldname)||!valid_name(newname))return 0;char a[128],b[128];join(a,sizeof(a),cwd,oldname);join(b,sizeof(b),cwd,newname);if(fs_exists(b))return 0;Overlay*e=ov_find(a);if(e&&e->type==FS_FILE){Overlay*n=ov_alloc(b,FS_FILE);if(!n)return 0;kmemcpy(n->data,e->data,e->size);n->size=e->size;e->type=OV_WHITEOUT;persist_slot(ov_index(n));persist_slot(ov_index(e));return 1;}u32 sz=0;const u8*d=fs_read_path(a,&sz);if(d&&sz<RAM_FILE_SIZE){Overlay*n=ov_alloc(b,FS_FILE);if(!n)return 0;kmemcpy(n->data,d,sz);n->size=sz;if(sz<RAM_FILE_SIZE)n->data[sz]=0;Overlay*w=ov_alloc(a,OV_WHITEOUT);persist_slot(ov_index(n));persist_slot(ov_index(w));return 1;}return 0;}
const char*fs_type_name(FsType t){switch(t){case FS_DIR:return"DIR";case FS_FILE:return"FILE";case FS_APP:return"APP";case FS_DOS:return"DOS";case FS_IMAGE:return"IMG";case FS_AUDIO:return"AUDIO";case FS_VIDEO:return"VIDEO";default:return"?";}}

int fs_persistent(void){return persist_on;}
