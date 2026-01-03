#include <kernel/heap.h>
#include <kernel/heap_internal.h>
#include <kernel/misc.h>
#include <kernel/utils.h>
#include <stddef.h>
#include <string.h>

#define ALIGNMENT 8
#define ALIGN(size) (((size) + (ALIGNMENT - 1)) & ~(ALIGNMENT - 1))

internal void heap_block_split(struct HeapBlock *block, size_t size);
internal void heap_block_merge_down(struct HeapBlock *block);
internal size_t heap_block_get_available_space(struct HeapBlock *block);

void *kmalloc(size_t size)
{
	size_t aligned_size = ALIGN(size);

	struct HeapBlock *node = heap_start;
	while (!node->EOM) {
		size_t available_space = heap_block_get_available_space(node);

		if (node->free && available_space >= aligned_size) {
			heap_block_split(node, aligned_size);
			node->free = HEAP_BLOCK_USED;
			return node + 1; // return memory within heap block
		}

		node = node->next;
	}
	// more allocation needed
	void *new_block = sbrk(aligned_size + sizeof(struct HeapBlock));
	if (new_block == (void *)-1) {
		KERNEL_DEBUG_LOGGER("sbrk() failed for size %d",
				    (int32_t)aligned_size);
		return NULL;
	}

	// at this point, node is the old EOM block. start of the newly allocated block
	internal_heap_block_set_metadata(
		node, EOM_FALSE, HEAP_BLOCK_USED,
		(struct HeapBlock *)program_break_point - 1);

	struct HeapBlock *new_heap_end =
		(struct HeapBlock *)program_break_point - 1;
	internal_heap_block_set_metadata(new_heap_end, EOM_TRUE,
					 HEAP_BLOCK_USED, NULL);

	node->next = new_heap_end;

	return node + 1;
}

void *kcalloc(size_t nmemb, size_t size)
{
	size_t bytes = nmemb * size;

	void *new_block = kmalloc(bytes);
	if (!new_block) {
		return NULL;
	}

	memset(new_block, 0, bytes);
	return new_block;
}

void *krealloc(void *ptr, size_t size)
{
	if (!ptr) {
		return kmalloc(size);
	}

	if (size == 0) {
		kfree(ptr);
		return NULL;
	}

	struct HeapBlock *original_block = (struct HeapBlock *)ptr - 1;

	size_t available_space = heap_block_get_available_space(original_block);
	if (available_space >= size) {
		heap_block_split(original_block, size);
		return ptr;
	}

	void *new_block = kmalloc(size);
	if (!new_block) {
		return NULL;
	}

	memmove(new_block, ptr, available_space);
	kfree(ptr);
	return new_block;
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
	size_t aligned_size = ALIGN(size);
	size_t available_space = heap_block_get_available_space(block);

	// no need to split
	if (available_space < aligned_size + sizeof(struct HeapBlock) +
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

internal size_t heap_block_get_available_space(struct HeapBlock *block)
{
	if (block->EOM) {
		return 0;
	}

	return (uintptr_t)block->next - (uintptr_t)block -
	       sizeof(struct HeapBlock);
}
