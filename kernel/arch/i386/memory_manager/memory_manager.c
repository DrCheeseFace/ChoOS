#include <kernel/memory_manager.h>
#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/utils.h>
#include <stddef.h>
#include <string.h>

extern void _tlb_flush(uint32_t addr);

int vmm_map_page(paddr_t phys, vaddr_t virt, uint32_t flags)
{
	page_t *page_directory_entry = (page_t *)GET_PDE_PTR(virt);

	// create table
	if (!page_directory_entry->present) {
		page_t *new_frame = kmalloc_page();
		if (new_frame == NULL) {
			return ENOMEM;
		}

		page_directory_entry->frame =
			((uintptr_t)new_frame) >> PAGE_SHIFT;
		page_directory_entry->present = 1;
		page_directory_entry->rw = 1;
		page_directory_entry->user = 1;

		uintptr_t page_table_virt_start =
			(uintptr_t)GET_PTE_PTR(virt) & PAGE_MASK;
		_tlb_flush(page_table_virt_start);
		memset((void *)page_table_virt_start, 0, PAGE_SIZE);
	}

	page_t *page_table_entry = (page_t *)GET_PTE_PTR(virt);
	page_table_entry->frame = phys >> PAGE_SHIFT;
	page_table_entry->present = 1;
	page_table_entry->rw = (flags & 0x2) ? 1 : 0;
	page_table_entry->user = (flags & 0x4) ? 1 : 0;

#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER("Mapped Virt 0x%x to Phys 0x%x", virt, phys);
#endif

	_tlb_flush(virt);
	return 0;
}

int vmm_unmap_page(vaddr_t virt)
{
	page_t *page_directory_entry = (page_t *)GET_PDE_PTR(virt);
	if (!page_directory_entry->present) {
#ifdef DEBUG_LOGGING
		KERNEL_DEBUG_LOGGER(
			"vmm_unmap_page failed. page directory entry not present for Virt 0x%x",
			virt);
#endif
		return -1;
	}

	page_t *page_table_entry = (page_t *)GET_PTE_PTR(virt);
	if (!page_table_entry->present) {
#ifdef DEBUG_LOGGING
		KERNEL_DEBUG_LOGGER("Virt 0x%x already not present", virt);
#endif
		return 0;
	}

#ifdef DEBUG_LOGGING
	uintptr_t phys = page_table_entry->frame << 12;
#endif
	page_table_entry->present = 0;
	page_table_entry->frame = 0;

#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER("Unmapped Virt 0x%x from Phys 0x%x", virt, phys);
#endif

	_tlb_flush(virt);
	return 0;
}
