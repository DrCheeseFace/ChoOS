#include <kernel/utils.h>
#include <stdint.h>

#include <kernel/misc.h>
#include <kernel/paging.h>
#include <kernel/paging_internal.h>

global_variable Page *pre_frames[BATCH_PAGES_ALLOCED_MAX];

void pmm_free_page(Page *a)
{
	paddr_t phys_addr = (paddr_t)a;
	uint32_t index = phys_addr / PAGE_SIZE;
	internal_pafe_frames_state_bitmap_unset(index);
}

Page *pmm_alloc_page(void)
{
	local_persist uint8_t allocate = 1;
	local_persist uint8_t pframe = 0;
	Page *ret;

	if (pframe == BATCH_PAGES_ALLOCED_MAX) {
		allocate = 1;
	}

	if (allocate == 1) {
		for (int i = 0; i < BATCH_PAGES_ALLOCED_MAX; i++) {
			Page *frame = pmm_alloc_frame_int();
			if (frame == NULL) {
				return NULL;
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

Page *pmm_alloc_frame_int(void)
{
	for (size_t i = 0; i < BITMAP_SIZE; i++) {
		if (page_frames_state_bitmap[i] != 0xFFFFFFFF) {
			for (int j = 0; j < 32; j++) {
				int bit_idx = i * 32 + j;
				if (!internal_page_frames_state_bitmap_test(
					    bit_idx)) {
					internal_page_frames_state_bitmap_set(
						bit_idx);

					paddr_t frame_phys_addr =
						bit_idx * PAGE_SIZE;
					return (void *)frame_phys_addr;
				}
			}
		}
	}

	KERNEL_DEBUG_LOGGER("WARNING: run out of page frames");
	return NULL;
}
