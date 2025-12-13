
#ifndef _STD_INTERNEL_H
#define _STD_INTERNEL_H

#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>

#include <sys/cdefs.h>

#define EOF (-1)
#define EOVERFLOW (75)

#ifdef __cplusplus
extern "C" {
#endif

bool __print(const char *data, size_t length);
void __swap(char *a, char *b);
void __reverse(char *str, int length);
void __utoa(unsigned int value, char *buffer, int base);
char *__itoa(int num, char *str, int base);

#ifdef __cplusplus
}
#endif

#endif
