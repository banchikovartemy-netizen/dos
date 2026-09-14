#include "lib.h"

usize kstrlen(const char *s) {
    usize n = 0;
    while (s && s[n]) n++;
    return n;
}

int kstrcmp(const char *a, const char *b) {
    while (*a && (*a == *b)) { a++; b++; }
    return (unsigned char)*a - (unsigned char)*b;
}

int kstrncmp(const char *a, const char *b, usize n) {
    while (n && *a && (*a == *b)) { a++; b++; n--; }
    if (!n) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

void *kmemset(void *dst, int value, usize n) {
    u8 *d = (u8*)dst;
    for (usize i = 0; i < n; i++) d[i] = (u8)value;
    return dst;
}

void *kmemcpy(void *dst, const void *src, usize n) {
    u8 *d = (u8*)dst;
    const u8 *s = (const u8*)src;
    for (usize i = 0; i < n; i++) d[i] = s[i];
    return dst;
}

void kstrcpy(char *dst, const char *src) {
    while ((*dst++ = *src++));
}

void kstrncpy(char *dst, const char *src, usize n) {
    usize i = 0;
    for (; i + 1 < n && src[i]; i++) dst[i] = src[i];
    if (n) dst[i] = 0;
}


void kstrcat(char *dst,const char *src,usize n){usize d=kstrlen(dst);if(d>=n)return;kstrncpy(dst+d,src,n-d);}
const char *kstrchr(const char *s,char c){while(*s){if(*s==c)return s;s++;}return 0;}

char kupper(char c) {
    if (c >= 'a' && c <= 'z') return (char)(c - ('a' - 'A'));
    return c;
}

int kisdigit(char c) { return c >= '0' && c <= '9'; }

i32 katoi(const char *s) {
    i32 sign = 1, v = 0;
    if (*s == '-') { sign = -1; s++; }
    while (kisdigit(*s)) { v = v * 10 + (*s - '0'); s++; }
    return v * sign;
}

void kitoa(i32 value, char *out) {
    char tmp[16];
    int i = 0, j = 0;
    if (value == 0) { out[0] = '0'; out[1] = 0; return; }
    if (value < 0) { out[j++] = '-'; value = -value; }
    while (value && i < 15) { tmp[i++] = (char)('0' + value % 10); value /= 10; }
    while (i) out[j++] = tmp[--i];
    out[j] = 0;
}

u32 khash(const char *s) {
    u32 h = 2166136261u;
    while (*s) { h ^= (u8)*s++; h *= 16777619u; }
    return h;
}
