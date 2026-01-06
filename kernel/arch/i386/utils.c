#include <kernel/heap.h>
#include <kernel/heap_internal.h>
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
	printf("%s:%d: ", filename, line);

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);

	printf("\n");
}

void kernel_debug_log_heap_info(const char *filename, int line)
{
	printf("%s:%d: logged heap\n", filename, line);
	printf("------HEAPBLOCKS------\n");
	struct HeapBlock *node = heap_start;
	int i = 0;
	while (!node->EOM) {
		printf("idx: %d; ptr: 0x%x; free: %s; available space: %d\n", i,
		       node, node->free ? "FREE" : "USED",
		       internal_heap_block_get_available_space(node));

		i++;
		node = node->next;
	}
	printf("----END-HEAPBLOCKS----\n");
}
