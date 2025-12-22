#include <kernel/paging_internal.h>
#include <stdint.h>

#define INDEX_FROM_BIT(a) (a / 32)
#define OFFSET_FROM_BIT(a) (a % 32)

void internal_page_frames_state_bitmap_set(uint32_t bit)
{
	page_frames_state_bitmap[INDEX_FROM_BIT(bit)] |=
		(1 << OFFSET_FROM_BIT(bit));
}

void internal_pafe_frames_state_bitmap_unset(uint32_t bit)
{
	page_frames_state_bitmap[INDEX_FROM_BIT(bit)] &=
		~(1 << OFFSET_FROM_BIT(bit));
}

bool internal_page_frames_state_bitmap_test(uint32_t bit)
{
	return page_frames_state_bitmap[INDEX_FROM_BIT(bit)] &
	       (1 << OFFSET_FROM_BIT(bit));
}
