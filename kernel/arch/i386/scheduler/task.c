#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/heap_internal.h>
#include <kernel/idt.h>
#include <kernel/paging.h>
#include <kernel/task.h>
#include <kernel/task_internal.h>
#include <kernel/timer.h>
#include <kernel/utils.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

global_variable uint64_t last_tick;
global_variable uint64_t current_pid = 0;
volatile uint32_t IRQ_disable_counter = 0;
volatile uint32_t postpone_task_switches_counter = 0;
volatile uint8_t task_switches_postponed_flag = 0;

volatile struct ProcessControlBlock *current_task_PCB = NULL;
volatile struct ProcessControlBlock *first_ready_to_run_task = NULL;
volatile struct ProcessControlBlock *last_ready_to_run_task = NULL;
volatile struct ProcessControlBlock *sleeping_taskes = NULL;
volatile struct ProcessControlBlock *dead_tasks = NULL;

internal void update_time_used(void);
internal void free_task(volatile struct ProcessControlBlock *node);
internal void mark_task_for_death(void);
internal void task_start_up(void);

void multitasking_initialize(void)
{
	KERNEL_DEBUG_LOGGER("init multitasking");

	__asm__ volatile("cli");

	struct ProcessControlBlock *kernel_task = kmalloc(sizeof(*kernel_task));
	if (!kernel_task) {
		KERNEL_DEBUG_LOGGER("failed to allocate kernel task");
		abort("failed to allocate kernel task");
	}
	memset(kernel_task, 0, sizeof(*kernel_task));

	uint32_t cr3 = _read_cr3();
	kernel_task->cr3 = cr3;
	kernel_task->next = kernel_task;
	current_task_PCB = kernel_task;
	kernel_task->state = TASK_STATE_RUNNING;
	kernel_task->id = current_pid;
	kernel_task->stack_base = NULL;

	last_tick = read_ticks_since_boot();

	install_timer_event(&internal_update_awoken_taskes);

	postpone_task_switches_counter = 0;
	task_switches_postponed_flag = 0;
	IRQ_disable_counter = 0;

	__asm__ volatile("sti");

	KERNEL_DEBUG_LOGGER("init multitasking OK");
}

void dead_tasks_cleanup(void)
{
	lock_scheduler();

	volatile struct ProcessControlBlock *node = dead_tasks;
	while (node) {
		volatile struct ProcessControlBlock *next = node->next;
		free_task(node);
		node = next;
	}
	dead_tasks = NULL;

	unlock_scheduler();
}

PID create_kernel_task(void (*func)(void))
{
	lock_scheduler();
	current_pid++;

	struct ProcessControlBlock *new_task = kmalloc(sizeof(*new_task));
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

	new_task->esp = _forge_kernel_stack(stack_top, func, task_start_up,
					    mark_task_for_death);
	new_task->esp0 = stack_top;
	new_task->cr3 = _read_cr3();
	new_task->state = TASK_STATE_READY_TO_RUN;
	new_task->id = current_pid;
	new_task->stack_base = stack_base;

	if (!first_ready_to_run_task) {
		first_ready_to_run_task = new_task;
		last_ready_to_run_task = new_task;
	}
	else {
		last_ready_to_run_task->next = new_task;
		last_ready_to_run_task = new_task;
	}

	unlock_scheduler();
	return new_task->id;
}

void schedule(void)
{
	if (postpone_task_switches_counter != 0) {
		task_switches_postponed_flag = 1;
		return;
	}

	if (!first_ready_to_run_task)
		return;

	volatile struct ProcessControlBlock *next_task =
		first_ready_to_run_task;
	first_ready_to_run_task = next_task->next;

	if (!first_ready_to_run_task) {
		last_ready_to_run_task = NULL;
	}

	update_time_used();

	if (current_task_PCB->state == TASK_STATE_RUNNING ||
	    current_task_PCB->state == TASK_STATE_READY_TO_RUN) {
		current_task_PCB->state = TASK_STATE_READY_TO_RUN;
		current_task_PCB->next = NULL;

		if (last_ready_to_run_task) {
			last_ready_to_run_task->next = current_task_PCB;
			last_ready_to_run_task = current_task_PCB;
		}
		else {
			first_ready_to_run_task = current_task_PCB;
			last_ready_to_run_task = current_task_PCB;
		}
	}

	_switch_to_task(next_task);
}

void lock_scheduler(void)
{
	__asm__ volatile("cli");
	IRQ_disable_counter++;
	postpone_task_switches_counter++;
}

void unlock_scheduler(void)
{
	postpone_task_switches_counter--;
	if (postpone_task_switches_counter == 0) {
		if (task_switches_postponed_flag != 0) {
			task_switches_postponed_flag = 0;
			schedule();
		}
	}
	IRQ_disable_counter--;
	if (IRQ_disable_counter == 0) {
		__asm__ volatile("sti");
	}
}

int get_task_state(PID pid)
{
	if (current_task_PCB && current_task_PCB->id == pid) {
		return current_task_PCB->state;
	}

	// check ready to run tasks
	struct ProcessControlBlock *node = (void *)first_ready_to_run_task;
	while (node) {
		if (node->id == pid) {
			return node->state;
		}
		node = (void *)node->next;
	}

	// check sleeping tasks
	node = (void *)sleeping_taskes;
	while (node) {
		if (node->id == pid) {
			return node->state;
		}
		node = (void *)node->next;
	}

	// check dead/ terminated tasks
	node = (void *)dead_tasks;
	while (node) {
		if (node->id == pid) {
			return node->state;
		}
		node = (void *)node->next;
	}

	// task doesnt exist or already cleanedup
	return -1;
}

void block_task(int reason)
{
	lock_scheduler();

	current_task_PCB->state = reason;

	// other way aaround?
	unlock_scheduler();
	schedule();
}

void unblock_task(volatile struct ProcessControlBlock *task)
{
	lock_scheduler();

	task->state = TASK_STATE_READY_TO_RUN;
	task->next = NULL;

	if (first_ready_to_run_task == NULL) {
		first_ready_to_run_task = task;
		last_ready_to_run_task = task;
	}
	else {
		last_ready_to_run_task->next = task;
		last_ready_to_run_task = task;
	}

	unlock_scheduler();
}

void terminate_task(void)
{
	lock_scheduler();

	// push to front of dead queue
	current_task_PCB->next = dead_tasks;
	dead_tasks = current_task_PCB;

	unlock_scheduler();

	block_task(TASK_STATE_TERMINATED);

	abort("task terminate_task returned: no other taskes ready to run. ie: you fucked up");
}

uint64_t get_current_task_time_used(void)
{
	update_time_used();
	return current_task_PCB->time_used;
}

internal void update_time_used(void)
{
	uint64_t current_tick = read_ticks_since_boot();
	uint64_t elapsed = current_tick - last_tick;
	last_tick = current_tick;
	current_task_PCB->time_used += elapsed;
}

internal void free_task(volatile struct ProcessControlBlock *node)
{
	kfree((void *)(node->stack_base));
	kfree((void *)node);
}

internal void mark_task_for_death(void)
{
	lock_scheduler();

	current_task_PCB->state = TASK_STATE_DEAD;

	// push to front of dead queue
	current_task_PCB->next = dead_tasks;
	dead_tasks = current_task_PCB;

	unlock_scheduler();
	schedule();

	abort("task cleanup task returned: no other taskes ready to run. ie: you fucked up");
}

internal void task_start_up(void)
{
	__asm__ volatile("sti");
}
