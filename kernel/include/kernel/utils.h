#ifndef _KERNEL_UTILS_H
#define _KERNEL_UTILS_H

#include <stdint.h>

struct interrupt_resigters {
	uint32_t cr2;
	uint32_t ds;
	uint32_t edi, esi, ibp, esp, ebx, edx, ecx, eax;
	uint32_t int_no, err_code;
	uint32_t eip, csm, eflags, useresp, ss;
};

void outb(uint16_t port, uint8_t value);

uint8_t inb(uint16_t port);

void kernel_logger(const char *filename, int line, const char *format, ...);

#define KERNEL_DEBUG_LOGGER(fmt, ...)                                          \
	kernel_logger(__FILE__, __LINE__, fmt, ##__VA_ARGS__)

#endif
