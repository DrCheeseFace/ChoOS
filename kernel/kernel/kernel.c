#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/heap_internal.h>
#include <kernel/idt.h>
#include <kernel/keyboard.h>
#include <kernel/multiboot.h>
#include <kernel/paging.h>
#include <kernel/process.h>
#include <kernel/sleep.h>
#include <kernel/ssp.h>
#include <kernel/test.h>
#include <kernel/timer.h>
#include <kernel/tty.h>
#include <kernel/utils.h>
#include <kernel/vmm.h>
#include <stdio.h>

void test_sleep(void)
{
	for (int i = 0; i < 10; i++) {
		nano_sleep(100);
		printf("hi\n");
		schedule();
	}
}

void test_sleep1(void)
{
	for (int i = 0; i < 10; i++) {
		nano_sleep(200);
		printf("hello\n");
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
	multiprocessing_initialize();
	keyboard_init();
	test_all();

	unused uint32_t proc = create_kernel_process(test_sleep);
	unused uint32_t proc_1 = create_kernel_process(test_sleep1);
	while (get_process_state(proc) || get_process_state(proc_1)) {
		lock_scheduler();
		schedule();
		unlock_scheduler();
	}

	dead_processs_cleanup();

	while (get_process_state(0)) {
		lock_scheduler();
		schedule();
		unlock_scheduler();
	}
}
