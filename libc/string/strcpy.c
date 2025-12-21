#include <string.h>

char *strcpy(char *restrict dst, const char *restrict src)
{
	char *ret = dst;
	while ((*dst++ = *src++))
		;
	return ret;
}
