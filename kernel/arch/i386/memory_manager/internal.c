#include <kernel/heap.h>
#include <kernel/misc.h>
#include <stdbool.h>

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
