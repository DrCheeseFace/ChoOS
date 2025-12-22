#include <kernel/gdt.h>
#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/paging_internal.h>
#include <kernel/utils.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

global_variable Page *page_directory;
global_variable uintptr_t page_frames_start_addr;
global_variable size_t page_frames_len = 0;
global_variable uint64_t total_free_memory = 0;
uint32_t page_frames_state_bitmap[BITMAP_SIZE];

internal void pmm_frames_init(multiboot_info_t *mbd);

void pmm_directory_init(uint32_t magic, multiboot_info_t *mbd)
{
	KERNEL_DEBUG_LOGGER("init paging");
	if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
		abort("KMALLOC_FAILED_TO_ALLOCATE_ERR: invalid memory map given by grub bootloader");
	}

	if (!(mbd->flags >> 6 & 0x1)) {
		abort("KMALLOC_FAILED_TO_ALLOCATE_ERR: invalid memory map given by grub bootloader");
	}

	pmm_frames_init(mbd);

	Page *page_directory_phys = pmm_alloc_frame_int();
	if (page_directory_phys == NULL) {
		abort("KMALLOC_FAILED_TO_ALLOCATE_ERR: failed to allocate frame for page directory");
	}
	page_directory = P2V(page_directory_phys);
	memset(page_directory_phys, 0, PAGE_SIZE);

	Page *first_page_table = pmm_alloc_frame_int();
	if (first_page_table == NULL) {
		abort("KMALLOC_FAILED_TO_ALLOCATE_ERR: failed to allocate frame for first page table");
	}
	memset(first_page_table, 0, PAGE_SIZE);

	for (uint32_t i = 0; i < PAGES_PER_TABLE; i++) {
		first_page_table[i].frame = i;
		first_page_table[i].present = 1;
		first_page_table[i].rw = 1;
		first_page_table[i].user = 0;
	}

	page_directory_phys[0].frame =
		((uintptr_t)first_page_table) >> PAGE_SHIFT;
	page_directory_phys[0].present = 1;
	page_directory_phys[0].rw = 1;
	page_directory_phys[0].user = 0;

	page_directory_phys[768].frame =
		((uintptr_t)first_page_table) >> PAGE_SHIFT;
	page_directory_phys[768].present = 1;
	page_directory_phys[768].rw = 1;
	page_directory_phys[768].user = 0;

	page_directory_phys[1023].frame =
		((uintptr_t)page_directory_phys >> PAGE_SHIFT);
	page_directory_phys[1023].present = 1;
	page_directory_phys[1023].rw = 1;
	page_directory_phys[1023].user = 0;

	_loadPageDirectory((uintptr_t)page_directory_phys);

	KERNEL_DEBUG_LOGGER("init paging OK");
}

internal void pmm_frames_init(multiboot_info_t *mbd)
{
	memset(page_frames_state_bitmap, 0xFF,
	       sizeof(page_frames_state_bitmap));

	page_frames_start_addr = 0x0;
	page_frames_len = MAX_PAGE_FRAME_COUNT;

	KERNEL_DEBUG_LOGGER("Initialized %d frames as USED",
			    MAX_PAGE_FRAME_COUNT);

	mmap_entry_t *entry = (mmap_entry_t *)mbd->mmap_addr;
	while ((uint32_t)entry < mbd->mmap_addr + mbd->mmap_length) {
		if (entry->type == MULTIBOOT_MEMORY_AVAILABLE &&
		    entry->addr_high == 0) {
			uintptr_t start = (uintptr_t)entry->addr_low;
			uintptr_t length = (uintptr_t)entry->len_low;
			uintptr_t end = start + length;

			// align to page boundaries
			start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);
			end = end & ~(PAGE_SIZE - 1);
			KERNEL_DEBUG_LOGGER(
				"Found AVAILABLE RAM: 0x%x - 0x%x: %u bytes",
				start, end, length);

			for (uintptr_t addr = start; addr < end;
			     addr += PAGE_SIZE) {
				uint32_t page_idx = addr / PAGE_SIZE;
				if (page_idx < MAX_PAGE_FRAME_COUNT) {
					internal_pafe_frames_state_bitmap_unset(
						page_idx);
				}
			}
		}
		else {
			KERNEL_DEBUG_LOGGER(
				"Found RESERVED Region: 0x%x - 0x%x",
				entry->addr_low,
				entry->addr_low + entry->len_low);
		}

		entry = (mmap_entry_t *)((unsigned int)entry + entry->size +
					 sizeof(entry->size));
	}

	uintptr_t k_start = (uintptr_t)&_kernel_start;
	uintptr_t k_end = (uintptr_t)&_kernel_end - KERNEL_VIRT_OFFSET;

	uint32_t start_idx = k_start / PAGE_SIZE;
	uint32_t end_idx = (k_end / PAGE_SIZE) + 1;

	KERNEL_DEBUG_LOGGER(
		"Protecting Kernel Memory: 0x%x - 0x%x (Pages %d to %d)",
		k_start, k_end, start_idx, end_idx);
	for (uint32_t i = start_idx; i < end_idx; i++) {
		if (i < MAX_PAGE_FRAME_COUNT) {
			internal_page_frames_state_bitmap_set(i);
		}
	}

	KERNEL_DEBUG_LOGGER("Protecting Lower Memory");
	for (int i = 0; i < 256; i++) {
		internal_page_frames_state_bitmap_set(i);
	}

	int free_page_count = 0;
	for (uint32_t i = 0; i < BITMAP_SIZE; i++) {
		if (page_frames_state_bitmap[i] != 0xFFFFFFFF) {
			for (int j = 0; j < 32; j++) {
				if (!(page_frames_state_bitmap[i] & (1 << j))) {
					free_page_count++;
				}
			}
		}
	}

	total_free_memory = (uint64_t)free_page_count * PAGE_SIZE;

	KERNEL_DEBUG_LOGGER("Initialization Complete. Total Free Memory: %u MB",
			    (free_page_count * 4) / 1024);
}

uint64_t __get_total_free_memory(void)
{
	return total_free_memory;
}
