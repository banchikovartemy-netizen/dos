#ifndef PCOS_LIB_H
#define PCOS_LIB_H
#include "types.h"

usize kstrlen(const char *s);
int kstrcmp(const char *a, const char *b);
int kstrncmp(const char *a, const char *b, usize n);
void *kmemset(void *dst, int value, usize n);
void *kmemcpy(void *dst, const void *src, usize n);
void kstrcpy(char *dst, const char *src);
void kstrncpy(char *dst, const char *src, usize n);
void kstrcat(char *dst, const char *src, usize n);
const char *kstrchr(const char *s, char c);
char kupper(char c);
int kisdigit(char c);
i32 katoi(const char *s);
void kitoa(i32 value, char *out);
u32 khash(const char *s);

#endif
