#include <kernel/heap.h>
#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/utils.h>
#include <kernel/vmm.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct HeapBlock *heap_start = NULL;
uintptr_t program_break_point = 0;

internal void *increment_brk(uintptr_t increment);
internal void *decrement_brk(uintptr_t decrement);
internal void block_split(struct HeapBlock *block, size_t size);
internal void *block_set_metadata(struct HeapBlock *dst, bool EOM, bool free,
				  void *next);
internal void block_merge_down(struct HeapBlock *block);

void heap_init(void)
{
	KERNEL_DEBUG_LOGGER("init heap");

	uintptr_t virt_base = (uintptr_t)P2V(0);
	uintptr_t page_table_size = PAGE_SIZE * 1024; // 4MB
	uintptr_t hard_limit_addr = virt_base + page_table_size;

	uintptr_t heap_start_addr = (uintptr_t)&_kernel_end;
	if (heap_start_addr % PAGE_SIZE) {
		heap_start_addr = ((uintptr_t)&_kernel_end + PAGE_SIZE) &
				  ~(PAGE_SIZE - 1);
	}

	if (heap_start_addr >= hard_limit_addr) {
		abort("KERNEL TOO BIG FOR INITIAL PAGE TABLE");
	}
	program_break_point = hard_limit_addr;

	// init end block
	struct HeapBlock *heap_end =
		block_set_metadata((struct HeapBlock *)hard_limit_addr - 1,
				   EOM_TRUE, HEAP_BLOCK_USED, NULL);

	// init start block
	heap_start = block_set_metadata((void *)heap_start_addr, EOM_FALSE,
					HEAP_BLOCK_FREE, heap_end);

	KERNEL_DEBUG_LOGGER("heap start: 0x%x", heap_start_addr);
	KERNEL_DEBUG_LOGGER("init heap OK");
}

int brk(void *addr)
{
	intptr_t diff = (intptr_t)addr - program_break_point;
	void *res = sbrk(diff);
	if (res != (void *)-1) {
		return 0;
	}
	return -1;
}

void *sbrk(intptr_t increment)
{
	if (increment == 0) {
		return (void *)program_break_point;
	}
	if (increment > 0) {
		return increment_brk(increment);
	}
	return decrement_brk(-increment);
}

void *kmalloc(size_t size)
{
	struct HeapBlock *node = heap_start;
	while (!node->EOM) {
		size_t available_space =
			((uintptr_t)node->next -
			 ((uintptr_t)node - sizeof(struct HeapBlock)));

		if (node->free && available_space >= size) {
			block_split(node, size);
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

	block_set_metadata(node, EOM_FALSE, HEAP_BLOCK_USED,
			   (struct HeapBlock *)program_break_point - 1);

	struct HeapBlock *new_allocation =
		block_set_metadata(heap_end, EOM_TRUE, HEAP_BLOCK_USED, NULL);

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
		block_merge_down(heap_block);
	}

	if (heap_block == heap_start) {
		return;
	}

	struct HeapBlock *parent_block = heap_start;
	while (parent_block->next != heap_block) {
		parent_block = parent_block->next;
	}
	if (parent_block->free) {
		block_merge_down(parent_block);
	}
}

internal void block_merge_down(struct HeapBlock *block)
{
	block->next = block->next->next;
}

internal void block_split(struct HeapBlock *block, size_t size)
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
		block_set_metadata((void *)new_block_addr, EOM_FALSE,
				   HEAP_BLOCK_FREE, block->next);
	block->next = new_block;
}

internal void *block_set_metadata(struct HeapBlock *dst, bool EOM, bool free,
				  void *next)
{
	struct HeapBlock *heap_end = dst;
	heap_end->EOM = EOM;
	heap_end->free = free;
	heap_end->next = next;
	return heap_end;
}

internal void *increment_brk(uintptr_t increment)
{
	uintptr_t old_program_break = program_break_point;
	uintptr_t new_program_break = program_break_point + increment;

	uintptr_t current_mapped_top =
		(program_break_point + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	uintptr_t new_mapped_top =
		(new_program_break + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

	if (new_mapped_top > current_mapped_top) {
		uintptr_t vaddr = current_mapped_top;
		while (vaddr < new_mapped_top) {
			Page *page = pmm_alloc_page();
			if (page == NULL) {
				return (void *)-1;
			}

			int res = vmm_page_map((uintptr_t)page, vaddr, 0x3);
			if (res != 0) {
				KERNEL_DEBUG_LOGGER("failed to map VIRT 0x%x",
						    vaddr);
				pmm_free_page(page);
				// TODO unmap all the pages alloced
				return (void *)-1;
			}
			vaddr += PAGE_SIZE;
		}
	}
	program_break_point = new_program_break;

	KERNEL_DEBUG_LOGGER("new heap end %x", program_break_point);

	return (void *)old_program_break;
}

internal void *decrement_brk(uintptr_t decrement)
{
	uintptr_t old_program_break = program_break_point;
	uintptr_t new_program_break = program_break_point - decrement;

	if (new_program_break < (uintptr_t)heap_start) {
		return (void *)-1;
	}

	while (program_break_point - PAGE_SIZE >= new_program_break) {
		void *page_addr = (void *)(program_break_point - PAGE_SIZE);

		pmm_free_page(V2P(page_addr));
		int err = vmm_page_unmap((uintptr_t)page_addr);
		if (err != 0) {
			KERNEL_DEBUG_LOGGER(
				"failed to unmap PHYS 0x%x from VIRT 0x%x",
				(uintptr_t)V2P(page_addr),
				(uintptr_t)page_addr);
		}
		program_break_point -= PAGE_SIZE;
	}

	KERNEL_DEBUG_LOGGER("new heap end %x", program_break_point);

	return (void *)old_program_break;
}

// 1) start at head
//
// 2) is EOM header?
//     ?? some thingy here about merging the old heap_end block to its parent V
//     so memset old_heap_end to 0; old_heap_end_parent->next = new_heap_end
//     yes: allocate page(s). move heap_end block to end. if cannot allocate pages err: return OOM
//     no:
//
//     3) is free and has available space?
//      available_space = (address of next header - (address of this header - size of header))
//         yes: split if remaining
//         blocksize > sizeof(struct heap_block) + MIN_ALLOC_SIZE and return address
//         no: go to next heap block. back to step 2
