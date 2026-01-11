#include <kernel/memory_manager/heap.h>
#include <kernel/memory_manager/heap_internal.h>
#include <kernel/memory_manager/vmm.h>
#include <kernel/misc.h>
#include <kernel/paging/paging.h>
#include <kernel/utils.h>
#include <stdint.h>

internal void *increment_brk(size_t increment);
internal void *decrement_brk(size_t decrement);

int brk(vaddr_t addr)
{
	intptr_t diff = addr - program_break_point;
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

internal void *increment_brk(size_t increment)
{
	vaddr_t old_program_break = program_break_point;
	vaddr_t new_program_break = program_break_point + increment;

	vaddr_t current_mapped_top =
		(program_break_point + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
	vaddr_t new_mapped_top =
		(new_program_break + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

	if (new_mapped_top > current_mapped_top) {
		vaddr_t vaddr = current_mapped_top;
		while (vaddr < new_mapped_top) {
			Page *page = pmm_alloc_page();
			if (page == NULL) {
				return (void *)-1;
			}

			int res = vmm_page_map((paddr_t)page, vaddr, 0x3);
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

internal void *decrement_brk(size_t decrement)
{
	vaddr_t old_program_break = program_break_point;
	vaddr_t new_program_break = program_break_point - decrement;

	if (new_program_break < (vaddr_t)heap_start) {
		return (void *)-1;
	}

	while (program_break_point - PAGE_SIZE >= new_program_break) {
		vaddr_t page_addr = (program_break_point - PAGE_SIZE);

		pmm_free_page((void *)V2P(page_addr));
		int err = vmm_page_unmap(page_addr);
		if (err != 0) {
			KERNEL_DEBUG_LOGGER(
				"failed to unmap PHYS 0x%x from VIRT 0x%x",
				V2P(page_addr), page_addr);
		}
		program_break_point -= PAGE_SIZE;
	}

	KERNEL_DEBUG_LOGGER("new heap end %x", program_break_point);

	return (void *)old_program_break;
}
