#ifndef _KERNEL_PAGING_INTERNAL_H
#define _KERNEL_PAGING_INTERNAL_H

#include <kernel/paging/paging.h>
#include <stdbool.h>
#include <stdint.h>

#define BITMAP_SIZE (MAX_PAGE_FRAME_COUNT / 32)

extern uint32_t page_frames_state_bitmap[BITMAP_SIZE];

void internal_page_frames_state_bitmap_set(uint32_t bit);
void internal_pafe_frames_state_bitmap_unset(uint32_t bit);
bool internal_page_frames_state_bitmap_test(uint32_t bit);

Page *pmm_alloc_frame_int(void);

#endif // !_KERNEL_PAGING_INTERNAL_H
