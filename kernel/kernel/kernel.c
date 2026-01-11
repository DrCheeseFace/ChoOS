#include <kernel/gdt.h>
#include <kernel/interrupts/idt.h>
#include <kernel/interrupts/keyboard.h>
#include <kernel/interrupts/timer.h>
#include <kernel/memory_manager/heap.h>
#include <kernel/memory_manager/heap_internal.h>
#include <kernel/memory_manager/vmm.h>
#include <kernel/multiboot.h>
#include <kernel/paging/paging.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/scheduler/sleep.h>
#include <kernel/scheduler/task.h>
#include <kernel/ssp.h>
#include <kernel/test.h>
#include <kernel/tty.h>
#include <kernel/utils.h>
#include <stdio.h>

void test_sleep(void)
{
	for (int i = 0; i < 10; i++) {
		micro_sleep(100);
		printf("thread 1;\n");
		schedule();
	}
}

void test_sleep1(void)
{
	for (int i = 0; i < 10; i++) {
		micro_sleep(200);
		printf("thread 2;\n");
		if (i == 5) {
			terminate_task();
		}
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
	multitasking_initialize();
	keyboard_init();

	if (test_all()) {
		abort("tests failed");
	}

	unused uint32_t proc = create_kernel_task(test_sleep);
	unused uint32_t proc_1 = create_kernel_task(test_sleep1);

	while (get_task_state(proc) || get_task_state(proc_1)) {
		schedule();
		__asm__ volatile("hlt");
	}

	KERNEL_DEBUG_LOG_HEAP_INFO();
	dead_tasks_cleanup();
	KERNEL_DEBUG_LOG_HEAP_INFO();

	kernel_idle_task();
}

void kernel_idle_task(void)
{
	for (;;) {
		__asm__ volatile("hlt");
	}
}
