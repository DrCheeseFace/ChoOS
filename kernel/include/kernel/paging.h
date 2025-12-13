#ifndef _KERNEL_PAGING_H
#define _KERNEL_PAGING_H

#include <kernel/multiboot.h>
#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 0x1000 // 4096
#define MAX_PAGE_FRAME_COUNT 1024 * 1024

#define BATCH_PAGES_ALLOCED_MAX 20
#define PAGES_PER_TABLE 1024
#define PAGE_DIRECTORY_ENTRY_INIT 0x00000002

#define ERROR 0xCAFEBABE

typedef uintptr_t page_frame_t;

typedef uint8_t page_state_t;
#define PAGE_STATE_FREE 0
#define PAGE_STATE_USED 1

extern uint8_t endkernel[];
extern uint8_t startkernel[];

typedef multiboot_memory_map_t mmap_entry_t;

void paging_init(uint32_t magic, multiboot_info_t *mbd);

void kfree_frame(page_frame_t a);

page_frame_t kmalloc_frame(void);

extern void _loadPageDirectory(uint32_t addr);

extern void _enablePaging(void);

#endif
