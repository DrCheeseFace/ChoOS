#include <kernel/heap.h>
#include <kernel/misc.h>
#include <stdbool.h>

void *block_set_metadata(struct HeapBlock *dst, bool EOM, bool free, void *next)
{
	struct HeapBlock *heap_end = dst;
	heap_end->EOM = EOM;
	heap_end->free = free;
	heap_end->next = next;
	return heap_end;
}
