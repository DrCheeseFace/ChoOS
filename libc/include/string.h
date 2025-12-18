#ifndef _STRING_H
#define _STRING_H 1

#include <sys/cdefs.h>

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void *, const void *, size_t);
void *memcpy(void *restrict dstptr, const void *restrict srcptr, size_t size);
void *memmove(void *, const void *, size_t);
void *memset(void *, int, size_t);
void *memchr(const void *src, int c, size_t n);
size_t strlen(const char *);

#ifdef __cplusplus
}
#endif

#endif
