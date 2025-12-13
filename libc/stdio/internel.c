#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

bool __print(const char *data, size_t length)
{
	const unsigned char *bytes = (const unsigned char *)data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return FALSE;
	return TRUE;
}

void __swap(char *a, char *b)
{
	if (!a || !b)
		return;

	char temp = *(a);
	*(a) = *(b);
	*(b) = temp;
}

void __reverse(char *str, int length)
{
	int start = 0;
	int end = length - 1;
	while (start < end) {
		__swap((str + start), (str + end));
		start++;
		end--;
	}
}

void __utoa(unsigned int value, char *buffer, int base)
{
	char *ptr = buffer;
	char *low = buffer;

	do {
		unsigned int mod = value % base;

		*ptr++ = (mod < 10) ? (mod + '0') : (mod - 10 + 'a');

		value /= base;
	} while (value > 0);

	*ptr-- = '\0';

	while (low < ptr) {
		char tmp = *low;
		*low++ = *ptr;
		*ptr-- = tmp;
	}
}

char *__itoa(int num, char *str, int base)
{
	int i = 0;
	bool isNegative = FALSE;

	if (num == 0) {
		str[i++] = '0';
		str[i] = '\0';
		return str;
	}

	if (num < 0 && base == 10) {
		isNegative = TRUE;
		num = -num;
	}

	while (num != 0) {
		int rem = num % base;
		str[i++] = (rem > 9) ? (rem - 10) + 'a' : rem + '0';
		num = num / base;
	}

	if (isNegative == TRUE)
		str[i++] = '-';

	str[i] = '\0';
	__reverse(str, i);
	return str;
}
