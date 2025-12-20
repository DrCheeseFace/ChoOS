#ifndef _KERNEL_PAGING_H
#define _KERNEL_PAGING_H

#include <kernel/multiboot.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE (1UL << PAGE_SHIFT)
#define PAGE_MASK (~(PAGE_SIZE - 1))

#define MAX_PAGE_FRAME_COUNT 1024 * 1024
#define PAGE_DIRECTORY_LENGTH 1024

#define BATCH_PAGES_ALLOCED_MAX 20
#define PAGES_PER_TABLE 1024

#define KERNEL_VIRT_OFFSET 0xC0000000

#define P2V(addr) ((void *)((uintptr_t)(addr) + KERNEL_VIRT_OFFSET))
#define V2P(addr) ((void *)((uintptr_t)(addr) - KERNEL_VIRT_OFFSET))

typedef struct {
	uint32_t present : 1;
	uint32_t rw : 1;
	uint32_t user : 1;
	uint32_t w_through : 1;
	uint32_t cache_dis : 1;
	uint32_t accessed : 1;
	uint32_t dirty : 1;
	uint32_t pat : 1;
	uint32_t global : 1;
	uint32_t available : 3;
	uint32_t frame : 20;
} __attribute__((packed)) page_t;

extern uint8_t _kernel_end[];
extern uint8_t _kernel_start[];

typedef multiboot_memory_map_t mmap_entry_t;

void pmm_directory_init(uint32_t magic, multiboot_info_t *mbd);

void kfree_frame(page_t *a);

// returns page_t if ok
// returns NULL if ran out of memory
page_t *kmalloc_page(void);

uint64_t __get_total_free_memory(void);

extern void _loadPageDirectory(uint32_t addr);

extern void _enablePaging(void);

#endif
