#include <internel.h>
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

int vprintf(const char *restrict format, va_list parameters)
{
	int written = 0;
	char buffer[65];
	int err;

	while (*format != '\0') {
		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++; // skip the first % of %%

			size_t amount = 1;
			while (format[amount] && format[amount] != '%') {
				amount++;
			}

			if ((err = __append(format, amount, &written)) != 0)
				return err;

			format += amount;
			continue;
		}

		const char *format_begun_at = format++; // skip %

		if (*format == 'c') {
			format++;
			char c = (char)va_arg(parameters, int);
			if ((err = __append(&c, 1, &written)) != 0)
				return err;
		}
		else if (*format == 's') {
			format++;
			const char *str = va_arg(parameters, const char *);
			if ((err = __append(str, strlen(str), &written)) != 0)
				return err;
		}
		else if (*format == 'd') {
			format++;
			int i = va_arg(parameters, int);
			__itoa(i, buffer, 10);
			if ((err = __append(buffer, strlen(buffer),
					    &written)) != 0)
				return err;
		}
		else if (*format == 'u') {
			format++;
			unsigned int u = va_arg(parameters, unsigned int);
			__utoa(u, buffer, 10);
			if ((err = __append(buffer, strlen(buffer),
					    &written)) != 0)
				return err;
		}
		else if (*format == 'x') {
			format++;
			unsigned int u = va_arg(parameters, unsigned int);
			__utoa(u, buffer, 16);
			if ((err = __append(buffer, strlen(buffer),
					    &written)) != 0)
				return err;
		}
		else if (*format == 'l') {
			format++;
			if (*format == 'l') {
				format++;
				if (*format == 'u') {
					format++;
					unsigned long long u = va_arg(
						parameters, unsigned long long);
					__ulltoa(u, buffer, 10);
					if ((err = __append(buffer,
							    strlen(buffer),
							    &written)) != 0)
						return err;
				}
				else if (*format == 'd') {
					format++;
					long long d =
						va_arg(parameters, long long);
					__lltoa(d, buffer, 10);
					if ((err = __append(buffer,
							    strlen(buffer),
							    &written)) != 0)
						return err;
				}
			}
			else {
				// TODO: handle %l
				format = format_begun_at;
				if ((err = __append(format, strlen(format),
						    &written)) != 0)
					return err;
				format += strlen(format);
			}
		}
		else {
			// unknown specifier
			format = format_begun_at;
			if ((err = __append(format, strlen(format),
					    &written)) != 0)
				return err;
			format += strlen(format);
		}
	}

	va_end(parameters);
	return written;
}
