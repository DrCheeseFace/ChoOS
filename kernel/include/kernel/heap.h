#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define INITIAL_HEAP_SIZE 1024 * 1024

// TODO add magic padding
struct heap_block {
	struct heap_block *next;
#define EOM_TRUE 1
#define EOM_FALSE 0
	uint8_t EOM;
#define HEAP_BLOCK_FREE 1
#define HEAP_BLOCK_USED 0
	uint8_t free;
	uint8_t reserved[2]; // to make it 8 bytes
} __attribute__((packed));

void heap_init(void);

/*
 *
 * DESCRIPTION
 *     sbrk()  increments  the  program's data space by increment bytes.
 *     Calling sbrk() with an increment of 0 can be used to
 *     find the current location of the program break.
 *
 * RETURNS
 *     On success, returns the previous program break.
 *     (If the break was increased, then this value is  a  pointer  to
 *     the start of the newly allocated memory)
 *     On error, (void *) -1 is returned, and errno is set to ENOMEM.
 *
 */
void *sbrk(intptr_t increment);

#endif // !_KERNEL_HEAP_H
