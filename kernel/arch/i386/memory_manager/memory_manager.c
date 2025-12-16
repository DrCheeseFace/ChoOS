#include <kernel/memory_manager.h>
#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/utils.h>
#include <stddef.h>
#include <string.h>

extern void _tbl_flush(uint32_t addr);

void vmm_map_page(uint32_t phys, uint32_t virt, uint32_t flags)
{
	uint32_t page_directory_index = virt >> 22;

	uint32_t *page_directory_entry = GET_PDE_PTR(virt);

	// create table
	if ((*page_directory_entry & PAGE_NOT_PRESENT) == 0) {
		page_t frame = kmalloc_page();
		// TODO change this to user accessable, writeable later
		*page_directory_entry = frame | PERMISSION_PRESENT_RW;
#ifdef DEBUG_LOGGING
		KERNEL_DEBUG_LOGGER("flushing TBL");
#endif
		_tbl_flush(PT_BASE_VADDR + (page_directory_index << 12));
		uint32_t *pt_virtual =
			(uint32_t *)(PT_BASE_VADDR +
				     (page_directory_index << 12));
		memset(pt_virtual, 0, PAGE_SIZE);
	}

	uint32_t *page_table_entry = GET_PTE_PTR(virt);
	*page_table_entry = phys | flags;

#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER("flushing TBL");
#endif
	_tbl_flush(virt);
	return;
}
