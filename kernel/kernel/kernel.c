#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/keyboard.h>
#include <kernel/multiboot.h>
#include <kernel/paging.h>
#include <kernel/ssp.h>
#include <kernel/task.h>
#include <kernel/test.h>
#include <kernel/timer.h>
#include <kernel/tty.h>
#include <kernel/utils.h>
#include <kernel/vmm.h>

void kernel_main(uint32_t magic, multiboot_info_t *mbd)
{
#ifndef DEBUG
	KERNEL_DEBUG_LOGGER("entry: kernel_main");
#endif

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
