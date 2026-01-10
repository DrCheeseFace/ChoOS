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
		micro_sleep(100);
		printf("thread 1;");
		schedule();
	}
}

void test_sleep1(void)
{
	for (int i = 0; i < 10; i++) {
		micro_sleep(200);
		printf("thread 2;");
		schedule();
	}
}

void kernel_idle_task(void);

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

	if (test_all()) {
		abort("tests failed");
	}

	unused uint32_t proc = create_kernel_process(test_sleep);
	unused uint32_t proc_1 = create_kernel_process(test_sleep1);
	while (get_process_state(proc) || get_process_state(proc_1)) {
		lock_scheduler();
		schedule();
		unlock_scheduler();
	}

	KERNEL_DEBUG_LOG_HEAP_INFO();
	dead_processs_cleanup();
	KERNEL_DEBUG_LOG_HEAP_INFO();

	kernel_idle_task();
}

void kernel_idle_task(void)
{
	for (;;) {
		__asm__ volatile("hlt");
	}
}
