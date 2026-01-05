#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/idt.h>
#include <kernel/paging.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/utils.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

global_variable uint64_t last_tick;

volatile struct TaskControlBlock *current_task_TCB = NULL;
volatile struct TaskControlBlock *first_ready_to_run_task = NULL;
volatile struct TaskControlBlock *last_ready_to_run_task = NULL;

void task_start_up(void);
internal void update_time_used(void);

void multitasking_initialize(void)
{
	KERNEL_DEBUG_LOGGER("init multitasking");

	__asm__ volatile("cli");

	struct TaskControlBlock *kernel_task = kmalloc(sizeof(*kernel_task));
	if (!kernel_task) {
		KERNEL_DEBUG_LOGGER("failed to allocate kernel task");
		abort("failed to allocate kernel task");
	}
	memset(kernel_task, 0, sizeof(*kernel_task));

	uint32_t cr3 = _read_cr3();
	kernel_task->cr3 = cr3;
	kernel_task->next = kernel_task;
	current_task_TCB = kernel_task;
	kernel_task->state = TASK_STATE_RUNNING;

	last_tick = read_ticks_since_boot();

	__asm__ volatile("sti");

	KERNEL_DEBUG_LOGGER("init multitasking OK");
}

void create_kernel_task(void (*func)(void))
{
	struct TaskControlBlock *new_task = kmalloc(sizeof(*new_task));
	if (!new_task) {
		KERNEL_DEBUG_LOGGER("failed to allocate kernel task");
		abort("failed to allocate kernel task");
	}
	memset(new_task, 0, sizeof(*new_task));

	void *stack_base = kmalloc(KERNEL_STACK_SIZE);
	if (!stack_base) {
		kfree(new_task);
		KERNEL_DEBUG_LOGGER("failed to allocate task stack");
		abort("failed to allocate task stack");
	}

	uint32_t stack_top = (uint32_t)stack_base + KERNEL_STACK_SIZE;

	new_task->esp = _forge_kernel_stack(stack_top, func, task_start_up);
	new_task->esp0 = stack_top;
	new_task->cr3 = _read_cr3();
	new_task->state = TASK_STATE_READY_TO_RUN;

	if (!first_ready_to_run_task) {
		first_ready_to_run_task = new_task;
		last_ready_to_run_task = new_task;
	}
	else {
		last_ready_to_run_task->next = new_task;
		last_ready_to_run_task = new_task;
	}
}

void schedule(void)
{
	if (!first_ready_to_run_task)
		return;

	if (!first_ready_to_run_task->next)
		return;

	volatile struct TaskControlBlock *task_to_run = first_ready_to_run_task;
	first_ready_to_run_task = task_to_run->next;

	update_time_used();

	if (current_task_TCB->state == TASK_STATE_RUNNING) {
		current_task_TCB->state = TASK_STATE_READY_TO_RUN;
		last_ready_to_run_task->next = current_task_TCB;
		last_ready_to_run_task = current_task_TCB;
	}

	_switch_to_task(task_to_run);
}

void task_start_up(void)
{
	__asm__ volatile("sti");
}

uint64_t get_current_task_time_used(void)
{
	update_time_used();
	return current_task_TCB->time_used;
}

internal void update_time_used(void)
{
	uint64_t current_tick = read_ticks_since_boot();
	uint64_t elapsed = current_tick - last_tick;
	last_tick = current_tick;
	current_task_TCB->time_used += elapsed;
}
