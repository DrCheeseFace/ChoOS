#include <kernel/gdt.h>
#include <kernel/heap.h>
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
#include <stdio.h>

void test_1(void)
{
	for (size_t i = 0; i < 10; i++) {
		printf("TASK1111: %d\n", i);
		schedule();
	}
}

void test_2(void)
{
	for (size_t i = 100; i < 110; i++) {
		printf("TASK2222: %d\n", i);
		schedule();
	}
}

void kernel_main(uint32_t magic, multiboot_info_t *mbd)
{
	KERNEL_DEBUG_LOGGER("entry: kernel_main");

	tty_init();
	gdt_init();
	idt_init();
	pmm_directory_init(magic, mbd);
	heap_init();
	timer_init();
	keyboard_init();

	multitasking_initialize();

	test_all();

	create_kernel_task(test_1);

	test_2();

	printf("done done done");

	for (;;)
		;
}
