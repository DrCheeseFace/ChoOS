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

void internal_heap_log_info(void)
{
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

size_t internal_heap_block_get_available_space(struct HeapBlock *block)
{
	if (block->EOM) {
		return 0;
	}

	return (vaddr_t)block->next - (vaddr_t)block - sizeof(struct HeapBlock);
}
