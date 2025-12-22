#include <kernel/heap.h>
#include <kernel/heap_internal.h>
#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/utils.h>
#include <kernel/vmm.h>
#include <stdint.h>
#include <stdlib.h>

uintptr_t program_break_point = 0;
struct HeapBlock *heap_start = NULL;

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
		heap_block_set_metadata((struct HeapBlock *)hard_limit_addr - 1,
					EOM_TRUE, HEAP_BLOCK_USED, NULL);

	// init start block
	heap_start = heap_block_set_metadata((void *)heap_start_addr, EOM_FALSE,
					     HEAP_BLOCK_FREE, heap_end);

	KERNEL_DEBUG_LOGGER("heap start: 0x%x", heap_start_addr);
	KERNEL_DEBUG_LOGGER("init heap OK");
}
