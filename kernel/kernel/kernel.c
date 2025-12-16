#include "kernel/utils.h"
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/keyboard.h>
#include <kernel/memory_manager.h>
#include <kernel/multiboot.h>
#include <kernel/paging.h>
#include <kernel/ssp.h>
#include <kernel/timer.h>
#include <kernel/tty.h>

int test_all(void);
int test_vmm_aliasing(void);

void kernel_main(uint32_t magic, multiboot_info_t *mbd)
{
	tty_init();
	pmm_directory_init(magic, mbd);
	gdt_init();
	idt_init();
	timer_init();
	keyboard_init();

	test_all();

	for (;;)
		;
}

int test_all(void)
{
	KERNEL_DEBUG_LOGGER("TEST: STARTING TESTS");
	int err = 0;
	err = err || test_vmm_aliasing();
	return err;
}

int test_vmm_aliasing(void)
{
	KERNEL_DEBUG_LOGGER("TEST: VMM Aliasing... ");

	page_t *phys_frame = kmalloc_page();
	if (phys_frame == NULL) {
		KERNEL_DEBUG_LOGGER("failed to allcoated page");
	}

	uint32_t virt_a = 0xD0000000;
	uint32_t virt_b = 0xE0000000;

	vmm_map_page((paddr_t)phys_frame, virt_a, 3);
	vmm_map_page((paddr_t)phys_frame, virt_b, 3);

	uint32_t *ptr_a = (uint32_t *)virt_a;
	uint32_t *ptr_b = (uint32_t *)virt_b;

	*ptr_a = 0xDEADBEEF;

	if (*ptr_b == 0xDEADBEEF) {
		KERNEL_DEBUG_LOGGER("[PASSED]");
		KERNEL_DEBUG_LOGGER("   Writing to %x correctly updated %x",
				    virt_a, virt_b);
		return 0;
	}
	else {
		KERNEL_DEBUG_LOGGER("[FAILED]");
		KERNEL_DEBUG_LOGGER("   Expected 0xDEADBEEF but got %x",
				    *ptr_b);
		return 1;
	}
}
