#include <kernel/heap.h>
#include <kernel/paging.h>
#include <kernel/utils.h>
#include <kernel/vmm.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct heap_block *heap_start = NULL;
uintptr_t program_break_point = 0;

void heap_init(void)
{
#ifdef DEBUG
	KERNEL_DEBUG_LOGGER("init heap");
#endif

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
	uintptr_t heap_end_addr = hard_limit_addr;
	struct heap_block *heap_end =
		(struct heap_block *)(heap_end_addr -
				      sizeof(struct heap_block));
	heap_end->EOM = EOM_TRUE;
	heap_end->free = HEAP_BLOCK_USED;
	heap_end->next = NULL;

	// init start block
	heap_start = (struct heap_block *)heap_start_addr;
	heap_start->EOM = EOM_FALSE;
	heap_start->free = HEAP_BLOCK_FREE;
	heap_start->next = heap_end;

#ifdef DEBUG
	KERNEL_DEBUG_LOGGER("heap start: 0x%x", heap_start_addr);
	KERNEL_DEBUG_LOGGER("init heap OK");
#endif
}

void *sbrk(intptr_t increment)
{
	if (increment == 0) {
		return (void *)program_break_point;
	}

	struct heap_block *old_heap_end = heap_start->next;
	struct heap_block *old_heap_end_parent = heap_start;
	while (old_heap_end->EOM != EOM_TRUE) {
		old_heap_end_parent = old_heap_end_parent->next;
		old_heap_end = old_heap_end->next;
	}

	size_t required_size = increment + sizeof(struct heap_block);
	int pages_to_allocate =
		((required_size + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1)) /
		PAGE_SIZE;

	uintptr_t virt_addr = program_break_point;
	page_t *page = NULL;
	for (int i = 0; i < pages_to_allocate; i++) {
		page = kmalloc_page();
		if (page == NULL) {
			return (void *)-1;
		}

		int res = vmm_page_map((uintptr_t)page, virt_addr, 0x3);
		if (res != 0) {
			kfree_frame(page);
			return (void *)-1;
		}

		virt_addr += PAGE_SIZE;
	}

	uintptr_t new_heap_end_loc = virt_addr - sizeof(struct heap_block);

	memmove((void *)new_heap_end_loc, old_heap_end, sizeof(*old_heap_end));
	memset(old_heap_end, 0, sizeof(*old_heap_end));

	old_heap_end_parent->next = (struct heap_block *)new_heap_end_loc;

	program_break_point = virt_addr;

#ifdef DEBUG
	KERNEL_DEBUG_LOGGER("new heap end %x", program_break_point);
#endif

	return (void *)old_heap_end;
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
