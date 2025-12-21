#include <string.h>

char *strncpy(char *restrict dst, const char *restrict src, size_t dsize)
{
	size_t i;

	for (i = 0; i < dsize && src[i] != '\0'; i++) {
		dst[i] = src[i];
	}

	for (; i < dsize; i++) {
		dst[i] = '\0';
	}

	return dst;
}
