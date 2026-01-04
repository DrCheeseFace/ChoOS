#ifndef _KERNEL_VMM_H
#define _KERNEL_VMM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef uintptr_t paddr_t;
typedef uintptr_t vaddr_t;
#define ENOMEM 12

#define PT_BASE_VADDR 0xFFC00000 // start of the recursive area
#define PD_BASE_VADDR                                                          \
	0xFFFFF000 // start of the page directory mapped as a table

// Get a pointer to a specific Page Table Entry
// Usage: uint32_t *page_table_entry = GET_PTE_PTR(virt);
// Returns a pointer you can read/write to modify the mapping for 'addr'
#define GET_PTE_PTR(addr)                                                      \
	((uint32_t *)(PT_BASE_VADDR + (((uint32_t)(addr) >> 12) * 4)))

// Get a pointer to a specific Page Directory Entry
// Usage: uint32_t *page_directory_entry = GET_PDE_PTR(virt);
// Returns a pointer you can read/write to add/remove entire Page Tables
#define GET_PDE_PTR(addr)                                                      \
	((uint32_t *)(PD_BASE_VADDR + (((uint32_t)(addr) >> 22) * 4)))

extern void _tlb_flush(uint32_t addr);

// Returns 0 if ok
// Returns ENOMEM if run out of memory
int vmm_page_map(paddr_t phys, vaddr_t virt, uint32_t flags);

// Returns 0 if ok
// Returns -1 if page directory entry not found
int vmm_page_unmap(vaddr_t virt);

// Returns 0 if ok
// Returns ENOMEM if run out of memory
int vmm_page_map_range(paddr_t phys_start, vaddr_t virt_start, size_t count,
		       uint32_t flags);

paddr_t vmm_virt_to_phys(vaddr_t virt);

#endif //!_KERNEL_VMM_H
