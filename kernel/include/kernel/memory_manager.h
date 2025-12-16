#ifndef _KERNEL_MEMORY_MANAGER_H
#define _KERNEL_MEMORY_MANAGER_H

#include <stdbool.h>
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

// Returns 0 if ok
// Returns ENOMEM if run out of memory
int vmm_map_page(paddr_t phys, vaddr_t virt, uint32_t flags);

#endif
