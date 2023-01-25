#ifndef KSTRING_H
#define KSTRING_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef kroundup32
#    define kroundup32(x) (--(x), (x) |= (x) >> 1, (x) |= (x) >> 2, (x) |= (x) >> 4, (x) |= (x) >> 8, (x) |= (x) >> 16, ++(x))
#endif

typedef struct __kstring_t {
    size_t l, m;
    char* s;
} kstring_t;

#ifdef __cplusplus
extern "C" {
#endif

int ksprintf(kstring_t* s, const char* fmt, ...);
int ksplit_core(char* s, int delimiter, int* _max, int** _offsets);
void* kmemmem(const void* _str, int n, const void* _pat, int m, int** _prep);

#ifdef __cplusplus
}
#endif

static inline int kputsn(const char* p, int l, kstring_t* s) {
    if (s->l + l + 1 >= s->m) {
        s->m = s->l + l + 2;
        kroundup32(s->m);
        s->s = (char*)realloc(s->s, s->m);
    }
    memcpy(s->s + s->l, p, l);
    s->l += l;
    s->s[s->l] = 0;
    return l;
}

static inline int kputs(const char* p, kstring_t* s) {
    return kputsn(p, strlen(p), s);
}

static inline int kputc(int c, kstring_t* s) {
    if (s->l + 1 >= s->m) {
        s->m = s->l + 2;
        kroundup32(s->m);
        s->s = (char*)realloc(s->s, s->m);
    }
    s->s[s->l++] = c;
    s->s[s->l] = 0;
    return c;
}

static inline int kputw(int c, kstring_t* s) {
    char buf[16];
    int l, x;
    if (c == 0)
        return kputc('0', s);
    for (l = 0, x = c < 0 ? -c : c; x > 0; x /= 10)
        buf[l++] = x % 10 + '0';
    if (c < 0)
        buf[l++] = '-';
    if (s->l + l + 1 >= s->m) {
        s->m = s->l + l + 2;
        kroundup32(s->m);
        s->s = (char*)realloc(s->s, s->m);
    }
    for (x = l - 1; x >= 0; --x)
        s->s[s->l++] = buf[x];
    s->s[s->l] = 0;
    return 0;
}

static inline int kputuw(unsigned c, kstring_t* s) {
    char buf[16];
    int l, i;
    unsigned x;
    if (c == 0)
        return kputc('0', s);
    for (l = 0, x = c; x > 0; x /= 10)
        buf[l++] = x % 10 + '0';
    if (s->l + l + 1 >= s->m) {
        s->m = s->l + l + 2;
        kroundup32(s->m);
        s->s = (char*)realloc(s->s, s->m);
    }
    for (i = l - 1; i >= 0; --i)
        s->s[s->l++] = buf[i];
    s->s[s->l] = 0;
    return 0;
}

static inline int* ksplit(kstring_t* s, int delimiter, int* n) {
    int max = 0, *offsets = 0;
    *n = ksplit_core(s->s, delimiter, &max, &offsets);
    return offsets;
}

#endif
