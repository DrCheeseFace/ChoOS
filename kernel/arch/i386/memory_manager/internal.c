#include <kernel/heap.h>
#include <kernel/heap_internal.h>
#include <kernel/misc.h>

#include <stdbool.h>
#include <stdio.h>

struct HeapBlock *internal_heap_block_set_metadata(struct HeapBlock *dst,
						   bool EOM, bool free,
						   void *next)
{
	struct HeapBlock *heap_block = dst;
	heap_block->EOM = EOM;
	heap_block->free = free;
	heap_block->next = next;
	return heap_block;
}

size_t internal_heap_block_get_available_space(struct HeapBlock *block)
{
	if (block->EOM) {
		return 0;
	}

	return (vaddr_t)block->next - (vaddr_t)block - sizeof(struct HeapBlock);
}
