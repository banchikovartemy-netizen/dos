#ifndef PCOS_FS_H
#define PCOS_FS_H
#include "types.h"
typedef enum {FS_DIR,FS_FILE,FS_APP,FS_DOS,FS_IMAGE,FS_AUDIO,FS_VIDEO} FsType;
typedef struct {char name[48];FsType type;u32 size;} FsEntry;
void fs_init(u32 magic,u32 mbi_addr);
const FsEntry *fs_list(const char *path,int *count);
const u8 *fs_read_path(const char *path,u32 *size);
const u8 *fs_read(const char *cwd,const char *name,u32 *size);
const char *fs_cat(const char *path,const char *name);
int fs_cd(const char *cwd,const char *target,char *out,usize outn);
int fs_write_text(const char *path,const char *text);
int fs_touch(const char *cwd,const char *name);
int fs_mkdir(const char *cwd,const char *name);
int fs_remove(const char *cwd,const char *name);
int fs_rename(const char *cwd,const char *oldname,const char *newname);
int fs_exists(const char *path);
const char *fs_type_name(FsType t);
int fs_persistent(void);
#endif
