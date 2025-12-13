#include <kernel/gdt.h>
#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/utils.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

global_variable uint32_t *page_directory;
global_variable page_state_t page_frames_state[MAX_PAGE_FRAME_COUNT];
global_variable uintptr_t page_frames_start_addr;
global_variable size_t page_frames_len = 0;
global_variable page_frame_t pre_frames[BATCH_PAGES_ALLOCED_MAX];

internal void page_frames_init(multiboot_info_t *mbd);
internal page_frame_t kmalloc_frame_int(void);

void paging_init(uint32_t magic, multiboot_info_t *mbd)
{
#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER("init paging");
#endif
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		abort("KMALLOC_FAILED_TO_ALLOCATE_ERR: invalid memory map given by grub bootloader");
	}

	if (!(mbd->flags >> 6 & 0x1)) {
		abort("KMALLOC_FAILED_TO_ALLOCATE_ERR: invalid memory map given by grub bootloader");
	}

	page_frames_init(mbd);

	page_frame_t page_directory_phys = kmalloc_frame_int();
	if (page_directory_phys ==
	    (page_frame_t)KMALLOC_FAILED_TO_ALLOCATE_ERR) {
		abort("KMALLOC_FAILED_TO_ALLOCATE_ERR: failed to allocate frame for page directory");
	}
	page_directory = (uint32_t *)page_directory_phys;

	page_frame_t first_page_table_phys = kmalloc_frame_int();
	if (first_page_table_phys ==
	    (page_frame_t)KMALLOC_FAILED_TO_ALLOCATE_ERR) {
		abort("KMALLOC_FAILED_TO_ALLOCATE_ERR: failed to allocate frame for first page table");
	}
	uint32_t *first_page_table = (uint32_t *)first_page_table_phys;
	for (uint16_t i = 0; i < PAGES_PER_TABLE; i++) {
		first_page_table[i] = (i * PAGE_SIZE) | 3;
	}

	page_directory[0] = first_page_table_phys | 3;

	_loadPageDirectory(page_directory_phys);
	_enablePaging();

#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER("init paging OK");
#endif
}

internal void page_frames_init(multiboot_info_t *mbd)
{
	for (int i = 0; i < MAX_PAGE_FRAME_COUNT; i++) {
		page_frames_state[i] = PAGE_STATE_USED;
	}

	page_frames_start_addr = 0x0;
	page_frames_len = MAX_PAGE_FRAME_COUNT;

#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER("Initialized %d frames as USED",
			    MAX_PAGE_FRAME_COUNT);
#endif

	mmap_entry_t *entry = (mmap_entry_t *)mbd->mmap_addr;
	while ((uint32_t)entry < mbd->mmap_addr + mbd->mmap_length) {
		if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
			uintptr_t start = (uintptr_t)entry->addr_low;
			uintptr_t length = (uintptr_t)entry->len_low;
			uintptr_t end = start + length;
#ifdef DEBUG_LOGGING
			KERNEL_DEBUG_LOGGER(
				"Found AVAILABLE RAM: 0x%x - 0x%x: %u bytes",
				start, end, length);
#endif

			for (uintptr_t addr = start; addr < end;
			     addr += PAGE_SIZE) {
				uint32_t page_idx = addr / PAGE_SIZE;

				if (page_idx < MAX_PAGE_FRAME_COUNT) {
					page_frames_state[page_idx] =
						PAGE_STATE_FREE;
				}
			}
		}
		else {
#ifdef DEBUG_LOGGING
			KERNEL_DEBUG_LOGGER(
				"Found RESERVED Region: 0x%x - 0x%x",
				entry->addr_low,
				entry->addr_low + entry->len_low);
#endif
		}

		entry = (mmap_entry_t *)((unsigned int)entry + entry->size +
					 sizeof(entry->size));
	}

	uintptr_t k_start = (uintptr_t)&startkernel;
	;
	uintptr_t k_end = (uintptr_t)&endkernel;

	uint32_t start_idx = k_start / PAGE_SIZE;
	uint32_t end_idx = (k_end / PAGE_SIZE) + 1;

#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER(
		"Protecting Kernel Memory: 0x%x - 0x%x (Pages %d to %d)",
		k_start, k_end, start_idx, end_idx);
#endif

	for (uint32_t i = start_idx; i < end_idx; i++) {
		if (i < MAX_PAGE_FRAME_COUNT) {
			page_frames_state[i] = PAGE_STATE_USED;
		}
	}

#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER("Protecting Lower Memory");
#endif
	for (int i = 0; i < 256; i++) {
		page_frames_state[i] = PAGE_STATE_USED;
	}

#ifdef DEBUG_LOGGING
	int32_t free_count = 0;
	for (int i = 0; i < MAX_PAGE_FRAME_COUNT; i++) {
		if (page_frames_state[i] == PAGE_STATE_FREE)
			free_count++;
	}
	KERNEL_DEBUG_LOGGER("Initialization Complete. Total Free Memory: %u MB",
			    (free_count * 4) / 1024);
#endif
}

internal page_frame_t kmalloc_frame_int(void)
{
	uint32_t i = 0;
	while (page_frames_state[i] != PAGE_STATE_FREE) {
		i++;
		if (i == page_frames_len) {
#ifdef DEBUG_LOGGING
			KERNEL_DEBUG_LOGGER("WARNING: run out of page frames");
#endif
			return KMALLOC_FAILED_TO_ALLOCATE_ERR;
		}
	}
	page_frames_state[i] = PAGE_STATE_USED;
	return (page_frames_start_addr + (i * PAGE_SIZE));
}

void kfree_frame(page_frame_t a)
{
	page_frame_t offset = a - page_frames_start_addr;

	uint32_t index = ((uint32_t)offset) / PAGE_SIZE;

	if (index < page_frames_len) {
		page_frames_state[index] = PAGE_STATE_FREE;
	}
}

page_frame_t kmalloc_frame(void)
{
	local_persist uint8_t allocate = 1;
	local_persist uint8_t pframe = 0;
	page_frame_t ret;

	if (pframe == BATCH_PAGES_ALLOCED_MAX) {
		allocate = 1;
	}

	if (allocate == 1) {
		for (int i = 0; i < BATCH_PAGES_ALLOCED_MAX; i++) {
			page_frame_t frame = kmalloc_frame_int();
			if (frame ==
			    (page_frame_t)KMALLOC_FAILED_TO_ALLOCATE_ERR) {
				abort("KMALLOC_FAILED_TO_ALLOCATE_ERR: failed to allocate frame in kmalloc_frame");
			}
			pre_frames[i] = frame;
		}
		pframe = 0;
		allocate = 0;
	}
	ret = pre_frames[pframe];
	pframe++;
	return ret;
}
