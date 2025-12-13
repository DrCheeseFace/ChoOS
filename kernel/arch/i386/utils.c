#include <kernel/utils.h>
#include <stdarg.h>
#include <stdio.h>

void outb(uint16_t port, uint8_t value)
{
	__asm__ volatile("outb %1, %0" : : "dN"(port), "a"(value));
}

uint8_t inb(uint16_t port)
{
	uint8_t ret;
	__asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
	return ret;
}

void kernel_debug_logger(const char *filename, int line, const char *format,
			 ...)
{
	printf("[%s:%d]: ", filename, line);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}
