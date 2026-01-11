#include <kernel/memory_manager/vmm.h>
#include <kernel/misc.h>
#include <kernel/paging/paging.h>
#include <kernel/utils.h>
#include <stddef.h>
#include <string.h>

int vmm_page_map(paddr_t phys, vaddr_t virt, uint32_t flags)
{
	Page *page_directory_entry = (Page *)GET_PDE_PTR(virt);

	// create table
	if (!page_directory_entry->present) {
		Page *new_table_phys = pmm_alloc_page();
		if (!new_table_phys) {
			return ENOMEM;
		}
		page_directory_entry->frame =
			((paddr_t)new_table_phys) >> PAGE_SHIFT;
		page_directory_entry->present = 1;
		page_directory_entry->rw = 1;
		page_directory_entry->user = 1;

		vaddr_t page_table_virt_start =
			(uintptr_t)GET_PTE_PTR(virt) & PAGE_MASK;
		_tlb_flush(page_table_virt_start);
		memset((void *)page_table_virt_start, 0, PAGE_SIZE);
	}

	Page *page_table_entry = (Page *)GET_PTE_PTR(virt);
	page_table_entry->frame = phys >> PAGE_SHIFT;
	page_table_entry->present = 1;
	page_table_entry->rw = (flags & 0x2) ? 1 : 0;
	page_table_entry->user = (flags & 0x4) ? 1 : 0;

	_tlb_flush(virt);

	KERNEL_DEBUG_LOGGER("Mapped Virt 0x%x to Phys 0x%x", virt, phys);

	return 0;
}

int vmm_page_unmap(vaddr_t virt)
{
	Page *page_directory_entry = (Page *)GET_PDE_PTR(virt);
	if (!page_directory_entry->present) {
		KERNEL_DEBUG_LOGGER(
			"vmm_unmap_page failed. page directory entry not present for Virt 0x%x",
			virt);
		return -1;
	}

	Page *page_table_entry = (Page *)GET_PTE_PTR(virt);
	if (!page_table_entry->present) {
		KERNEL_DEBUG_LOGGER("Virt 0x%x already not present", virt);
		return 0;
	}

	paddr_t phys = page_table_entry->frame << 12;
	page_table_entry->present = 0;
	page_table_entry->frame = 0;

	KERNEL_DEBUG_LOGGER("Unmapped Virt 0x%x from Phys 0x%x", virt, phys);

	_tlb_flush(virt);
	return 0;
}

int vmm_page_map_range(paddr_t phys_start, vaddr_t virt_start, size_t count,
		       uint32_t flags)
{
	for (size_t i = 0; i < count; i++) {
		if (vmm_page_map(phys_start, virt_start, flags)) {
			return 1;
		}
	}

	return 0;
}

paddr_t vmm_virt_to_phys(vaddr_t virt)
{
	return V2P(virt);
}
