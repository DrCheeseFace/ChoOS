#ifndef _KERNEL_PAGING_H
#define _KERNEL_PAGING_H

#include <kernel/multiboot.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 0x1000 // 4096
#define MAX_PAGE_FRAME_COUNT 1024 * 1024
#define PAGE_DIRECTORY_LENGTH 1024

#define PERMISSION_PRESENT_RW 3
#define PAGE_NOT_PRESENT 0x1

#define BATCH_PAGES_ALLOCED_MAX 20
#define PAGES_PER_TABLE 1024

typedef uintptr_t page_t;

typedef uint8_t page_state_t;
#define PAGE_STATE_FREE 0
#define PAGE_STATE_USED 1

extern uint8_t endkernel[];
extern uint8_t startkernel[];

typedef multiboot_memory_map_t mmap_entry_t;

void pmm_directory_init(uint32_t magic, multiboot_info_t *mbd);

void kfree_frame(page_t a);

#define KMALLOC_FAILED_TO_ALLOCATE_ERR 0xCAFEBABE
page_t kmalloc_page(void);

extern void _loadPageDirectory(uint32_t addr);

extern void _enablePaging(void);

#endif
