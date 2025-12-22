#ifndef _KERNEL_UTILS_H
#define _KERNEL_UTILS_H

#include <stdint.h>

struct Registers {
	uint32_t cr2;
	uint32_t ds;
	// general purpose registers
	uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	// pointer register
	uint32_t eip, cs, eflags, useresp;
	// segment registers
	uint32_t ss;
};

void outb(uint16_t port, uint8_t value);

uint8_t inb(uint16_t port);

void kernel_debug_logger(const char *filename, int line, const char *format,
			 ...);

#ifdef DEBUG
#define KERNEL_DEBUG_LOGGER(fmt, ...)                                          \
	kernel_debug_logger(__FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
#define KERNEL_DEBUG_LOGGER(fmt, ...)                                          \
	do {                                                                   \
		if (0) {                                                       \
			kernel_debug_logger(__FILE__, __LINE__, fmt,           \
					    ##__VA_ARGS__);                    \
		}                                                              \
	} while (0)
#endif

#endif
