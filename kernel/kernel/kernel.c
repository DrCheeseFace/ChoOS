#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/heap_internal.h>
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
	for (int i = 0; i < 3; i++) {
		printf("TEST1 %d\n", i);
		schedule();
	}
}

void test_2(void)
{
	for (int i = 10; i < 13; i++) {
		printf("TEST2 %d\n", i);
		schedule();
	}
}

void test_3(void)
{
	for (int i = 100; i < 103; i++) {
		printf("TEST3 %d\n", i);
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
	multitasking_initialize();
	keyboard_init();

	test_all();

	create_kernel_task(test_1);
	create_kernel_task(test_2);
	test_3();

	internal_heap_log_info();

	for (;;)
		;
}
