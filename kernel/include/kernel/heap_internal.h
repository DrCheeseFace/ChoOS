#ifndef _KERNEL_HEAP_INTERNAL_H
#define _KERNEL_HEAP_INTERNAL_H

#include <kernel/vmm.h>
#include <stdbool.h>
#include <stdint.h>

extern vaddr_t program_break_point;
extern struct HeapBlock *heap_start;

struct HeapBlock *internal_heap_block_set_metadata(struct HeapBlock *dst,
						   bool EOM, bool free,
						   void *next);

#endif //! _KERNEL_HEAP_INTERNAL_H
