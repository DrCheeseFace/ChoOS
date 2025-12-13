#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

void swap(char *a, char *b)
{
	if (!a || !b)
		return;

	char temp = *(a);
	*(a) = *(b);
	*(b) = temp;
}

void reverse(char *str, int length)
{
	int start = 0;
	int end = length - 1;
	while (start < end) {
		swap((str + start), (str + end));
		start++;
		end--;
	}
}

char *itoa(int num, char *str, int base)
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
	reverse(str, i);
	return str;
}

void utoa(unsigned int value, char *buffer, int base)
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

static bool print(const char *data, size_t length)
{
	const unsigned char *bytes = (const unsigned char *)data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return FALSE;
	return TRUE;
}

int vprintf(const char *restrict format, va_list parameters)
{
	int written = 0;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				return EOVERFLOW;
			}
			if (!print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char *format_begun_at = format++;

		if (*format == 'c') {
			format++;
			char c = (char)va_arg(parameters,
					      int /* char promotes to int */);
			if (!maxrem) {
				return EOVERFLOW;
			}
			if (!print(&c, sizeof(c)))
				return -1;
			written++;
		}
		else if (*format == 's') {
			format++;
			const char *str = va_arg(parameters, const char *);
			size_t len = strlen(str);
			if (maxrem < len) {
				return EOVERFLOW;
			}
			if (!print(str, len))
				return -1;
			written += len;
		}
		else if (*format == 'd') {
			format++;
			int i = va_arg(parameters, int);
			char str[65];
			itoa(i, str, 10);
			size_t len = strlen(str);
			if (maxrem < len) {
				return EOVERFLOW;
			}
			if (!print(str, len))
				return -1;
			written += len;
		}
		else if (*format == 'u') {
			format++;
			unsigned int u = va_arg(parameters, unsigned int);
			char str[32];
			utoa(u, str, 10);
			size_t len = strlen(str);
			if (maxrem < len) {
				return EOVERFLOW;
			}
			if (!print(str, len))
				return -1;
			written += len;
		}
		else if (*format == 'x') {
			format++;
			unsigned int u = va_arg(parameters, unsigned int);
			char str[32];
			utoa(u, str, 16);
			size_t len = strlen(str);
			if (maxrem < len) {
				return EOVERFLOW;
			}
			if (!print(str, len))
				return -1;
			written += len;
		}
		else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				return EOVERFLOW;
			}
			if (!print(format, len))
				return -1;
			written += len;
			format += len;
		}
	}

	va_end(parameters);
	return written;
}

int printf(const char *restrict format, ...)
{
	va_list parameters;
	va_start(parameters, format);
	int written = vprintf(format, parameters);
	va_end(parameters);
	return written;
}
