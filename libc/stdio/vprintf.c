#include <internel.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

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
			if (!__print(format, amount))
				return -1;
			format += amount;
			written += amount;
			continue;
		}

		const char *format_begun_at = format++;

		if (*format == 'c') {
			format++;
			char c = (char)va_arg(parameters, int);
			if (!maxrem) {
				return EOVERFLOW;
			}
			if (!__print(&c, sizeof(c)))
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
			if (!__print(str, len))
				return -1;
			written += len;
		}
		else if (*format == 'd') {
			format++;
			int i = va_arg(parameters, int);
			char str[65];
			__itoa(i, str, 10);
			size_t len = strlen(str);
			if (maxrem < len) {
				return EOVERFLOW;
			}
			if (!__print(str, len))
				return -1;
			written += len;
		}
		else if (*format == 'u') {
			format++;
			unsigned int u = va_arg(parameters, unsigned int);
			char str[32];
			__utoa(u, str, 10);
			size_t len = strlen(str);
			if (maxrem < len) {
				return EOVERFLOW;
			}
			if (!__print(str, len))
				return -1;
			written += len;
		}
		else if (*format == 'x') {
			format++;
			unsigned int u = va_arg(parameters, unsigned int);
			char str[32];
			__utoa(u, str, 16);
			size_t len = strlen(str);
			if (maxrem < len) {
				return EOVERFLOW;
			}
			if (!__print(str, len))
				return -1;
			written += len;
		}
		else if (*format == 'l') {
			format++;
			if (*format == 'l') {
				format++;
				if (*format == 'u') {
					format++;
					unsigned long long u = va_arg(
						parameters, unsigned long long);
					char str[32];

					unsigned long long temp = u;
					int i = 0;
					if (temp == 0) {
						str[i++] = '0';
					}
					else {
						do {
							str[i++] =
								(char)((temp %
									10) +
								       '0');
							temp /= 10;
						} while (temp > 0);
					}

					for (int j = 0; j < i / 2; j++) {
						char tmp = str[j];
						str[j] = str[i - 1 - j];
						str[i - 1 - j] = tmp;
					}

					size_t len = i;
					if (maxrem < len) {
						return EOVERFLOW;
					}
					if (!__print(str, len))
						return -1;
					written += len;
				}
				else if (*format == 'd') {
					format++;
					long long d =
						va_arg(parameters, long long);
					char str[32];
					int i = 0;

					unsigned long long temp;
					if (d < 0) {
						if (!maxrem)
							return EOVERFLOW;
						char sign = '-';
						if (!__print(&sign, 1))
							return -1;
						written++;
						maxrem--;
						temp = (unsigned long long)(-(
							       d + 1)) +
						       1;
					}
					else {
						temp = (unsigned long long)d;
					}

					if (temp == 0) {
						str[i++] = '0';
					}
					else {
						do {
							str[i++] =
								(char)((temp %
									10) +
								       '0');
							temp /= 10;
						} while (temp > 0);
					}

					for (int j = 0; j < i / 2; j++) {
						char tmp = str[j];
						str[j] = str[i - 1 - j];
						str[i - 1 - j] = tmp;
					}

					size_t len = i;
					if (maxrem < len) {
						return EOVERFLOW;
					}
					if (!__print(str, len))
						return -1;
					written += len;
				}
			}
			else {
				// TODO
				// treat %lu as %u and %ld as %d for now
				format = format_begun_at;
				size_t len = strlen(format);
				if (maxrem < len)
					return EOVERFLOW;
				if (!__print(format, len))
					return -1;
				written += len;
				format += len;
			}
		}
		else {
			format = format_begun_at;
			size_t len = strlen(format);
			if (maxrem < len) {
				return EOVERFLOW;
			}
			if (!__print(format, len))
				return -1;
			written += len;
			format += len;
		}
	}

	va_end(parameters);
	return written;
}
