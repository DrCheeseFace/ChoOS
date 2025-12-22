#include <kernel/heap.h>
#include <kernel/heap_internal.h>
#include <kernel/misc.h>
#include <kernel/utils.h>
#include <stddef.h>
#include <string.h>

internal void heap_block_split(struct HeapBlock *block, size_t size);
internal void heap_block_merge_down(struct HeapBlock *block);

void *kmalloc(size_t size)
{
	struct HeapBlock *node = heap_start;
	while (!node->EOM) {
		size_t available_space =
			((uintptr_t)node->next -
			 ((uintptr_t)node - sizeof(struct HeapBlock)));

		if (node->free && available_space >= size) {
			heap_block_split(node, size);
			node->free = HEAP_BLOCK_USED;
			return node + 1; // return memory within heap block
		}
	}
	// more allocation needed
	void *new_block = sbrk(size + sizeof(struct HeapBlock));
	if (new_block == (void *)-1) {
		KERNEL_DEBUG_LOGGER("sbrk() failed for size %d", (int32_t)size);
		return NULL;
	}

	// at this point, node is the EOM block.
	struct HeapBlock *heap_end =
		(struct HeapBlock *)program_break_point - 1;
	memset((void *)node, 0, sizeof(struct HeapBlock));

	internal_heap_block_set_metadata(
		node, EOM_FALSE, HEAP_BLOCK_USED,
		(struct HeapBlock *)program_break_point - 1);

	struct HeapBlock *new_allocation = internal_heap_block_set_metadata(
		heap_end, EOM_TRUE, HEAP_BLOCK_USED, NULL);

	return new_allocation;
}

void kfree(void *ptr)
{
	struct HeapBlock *heap_block = (struct HeapBlock *)ptr - 1;
	if (heap_block->free) {
		KERNEL_DEBUG_LOGGER("DOUBLE FREE attempted at 0x%x", ptr);
	}

	heap_block->free = HEAP_BLOCK_FREE;

	if (heap_block->next->free) {
		heap_block_merge_down(heap_block);
	}

	if (heap_block == heap_start) {
		return;
	}

	struct HeapBlock *parent_block = heap_start;
	while (parent_block->next != heap_block) {
		parent_block = parent_block->next;
	}
	if (parent_block->free) {
		heap_block_merge_down(parent_block);
	}
}

internal void heap_block_merge_down(struct HeapBlock *block)
{
	block->next = block->next->next;
}

internal void heap_block_split(struct HeapBlock *block, size_t size)
{
	size_t available_space =
		((uintptr_t)block->next -
		 ((uintptr_t)block - sizeof(struct HeapBlock)));

	// no need to split
	if (available_space - size - sizeof(struct HeapBlock) <
	    MINIMUM_ALLOCATION_BYTES) {
		return;
	}

	uintptr_t new_block_addr =
		((uintptr_t)block + sizeof(struct HeapBlock) + size);

	struct HeapBlock *new_block =
		internal_heap_block_set_metadata((void *)new_block_addr,
						 EOM_FALSE, HEAP_BLOCK_FREE,
						 block->next);
	block->next = new_block;
}
