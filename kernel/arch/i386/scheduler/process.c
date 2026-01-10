#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/heap_internal.h>
#include <kernel/idt.h>
#include <kernel/paging.h>
#include <kernel/process.h>
#include <kernel/process_internal.h>
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

volatile struct ProcessControlBlock *current_process_PCB = NULL;
volatile struct ProcessControlBlock *first_ready_to_run_process = NULL;
volatile struct ProcessControlBlock *last_ready_to_run_process = NULL;
volatile struct ProcessControlBlock *sleeping_processes = NULL;
volatile struct ProcessControlBlock *dead_processs = NULL;

internal void update_time_used(void);
internal void free_process(volatile struct ProcessControlBlock *node);
internal void mark_process_for_death(void);
internal void process_start_up(void);

void multiprocessing_initialize(void)
{
	KERNEL_DEBUG_LOGGER("init multiprocessing");

	__asm__ volatile("cli");

	struct ProcessControlBlock *kernel_process =
		kmalloc(sizeof(*kernel_process));
	if (!kernel_process) {
		KERNEL_DEBUG_LOGGER("failed to allocate kernel process");
		abort("failed to allocate kernel process");
	}
	memset(kernel_process, 0, sizeof(*kernel_process));

	uint32_t cr3 = _read_cr3();
	kernel_process->cr3 = cr3;
	kernel_process->next = kernel_process;
	current_process_PCB = kernel_process;
	kernel_process->state = PROCESS_STATE_RUNNING;
	kernel_process->id = current_pid;
	kernel_process->stack_base = NULL;

	last_tick = read_ticks_since_boot();

	install_timer_event(&internal_update_awoken_processes);

	postpone_task_switches_counter = 0;
	task_switches_postponed_flag = 0;
	IRQ_disable_counter = 0;

	__asm__ volatile("sti");

	KERNEL_DEBUG_LOGGER("init multiprocessing OK");
}

void dead_processs_cleanup(void)
{
	lock_scheduler();

	volatile struct ProcessControlBlock *node = dead_processs;
	while (node) {
		volatile struct ProcessControlBlock *next = node->next;
		free_process(node);
		node = next;
	}
	dead_processs = NULL;

	unlock_scheduler();
}

PID create_kernel_process(void (*func)(void))
{
	lock_scheduler();
	current_pid++;

	struct ProcessControlBlock *new_process = kmalloc(sizeof(*new_process));
	if (!new_process) {
		KERNEL_DEBUG_LOGGER("failed to allocate kernel process");
		abort("failed to allocate kernel process");
	}
	memset(new_process, 0, sizeof(*new_process));

	void *stack_base = kmalloc(KERNEL_STACK_SIZE);
	if (!stack_base) {
		kfree(new_process);
		KERNEL_DEBUG_LOGGER("failed to allocate process stack");
		abort("failed to allocate process stack");
	}

	uint32_t stack_top = (uint32_t)stack_base + KERNEL_STACK_SIZE;

	new_process->esp = _forge_kernel_stack(
		stack_top, func, process_start_up, mark_process_for_death);
	new_process->esp0 = stack_top;
	new_process->cr3 = _read_cr3();
	new_process->state = PROCESS_STATE_READY_TO_RUN;
	new_process->id = current_pid;
	new_process->stack_base = stack_base;

	if (!first_ready_to_run_process) {
		first_ready_to_run_process = new_process;
		last_ready_to_run_process = new_process;
	}
	else {
		last_ready_to_run_process->next = new_process;
		last_ready_to_run_process = new_process;
	}

	unlock_scheduler();
	return new_process->id;
}

void schedule(void)
{
	if (postpone_task_switches_counter != 0) {
		task_switches_postponed_flag = 1;
		return;
	}

	if (!first_ready_to_run_process)
		return;

	volatile struct ProcessControlBlock *next_process =
		first_ready_to_run_process;
	first_ready_to_run_process = next_process->next;

	if (!first_ready_to_run_process) {
		last_ready_to_run_process = NULL;
	}

	update_time_used();

	if (current_process_PCB->state == PROCESS_STATE_RUNNING ||
	    current_process_PCB->state == PROCESS_STATE_READY_TO_RUN) {
		current_process_PCB->state = PROCESS_STATE_READY_TO_RUN;
		current_process_PCB->next = NULL;

		if (last_ready_to_run_process) {
			last_ready_to_run_process->next = current_process_PCB;
			last_ready_to_run_process = current_process_PCB;
		}
		else {
			first_ready_to_run_process = current_process_PCB;
			last_ready_to_run_process = current_process_PCB;
		}
	}

	_switch_to_process(next_process);
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

int get_process_state(PID pid)
{
	if (current_process_PCB && current_process_PCB->id == pid) {
		return current_process_PCB->state;
	}

	// check ready to run processes
	struct ProcessControlBlock *node = (void *)first_ready_to_run_process;
	while (node) {
		if (node->id == pid) {
			return node->state;
		}
		node = (void *)node->next;
	}

	// check sleeping processes
	node = (void *)sleeping_processes;
	while (node) {
		if (node->id == pid) {
			return node->state;
		}
		node = (void *)node->next;
	}

	// check dead processes
	node = (void *)dead_processs;
	while (node) {
		if (node->id == pid) {
			return node->state;
		}
		node = (void *)node->next;
	}

	// process doesnt exist or already cleanedup
	return -1;
}

void block_process(int reason)
{
	lock_scheduler();

	current_process_PCB->state = reason;
	current_process_PCB->next =
		(struct ProcessControlBlock *)sleeping_processes;
	sleeping_processes = current_process_PCB;

	unlock_scheduler();

	schedule();
}

void unblock_process(volatile struct ProcessControlBlock *process)
{
	lock_scheduler();

	if (sleeping_processes == process) {
		sleeping_processes = process->next;
	}
	else {
		volatile struct ProcessControlBlock *prev = sleeping_processes;
		while (prev && prev->next != process) {
			prev = prev->next;
		}
		if (prev) {
			prev->next = process->next;
		}
	}

	process->state = PROCESS_STATE_READY_TO_RUN;
	process->next = NULL;

	if (first_ready_to_run_process == NULL) {
		first_ready_to_run_process = process;
		last_ready_to_run_process = process;
	}
	else {
		last_ready_to_run_process->next = process;
		last_ready_to_run_process = process;
	}

	unlock_scheduler();
}

uint64_t get_current_process_time_used(void)
{
	update_time_used();
	return current_process_PCB->time_used;
}

internal void update_time_used(void)
{
	uint64_t current_tick = read_ticks_since_boot();
	uint64_t elapsed = current_tick - last_tick;
	last_tick = current_tick;
	current_process_PCB->time_used += elapsed;
}

internal void free_process(volatile struct ProcessControlBlock *node)
{
	kfree((void *)(node->stack_base));
	kfree((void *)node);
}

internal void mark_process_for_death(void)
{
	lock_scheduler();

	current_process_PCB->state = PROCESS_STATE_DEAD;

	// push to front of dead queue
	current_process_PCB->next = dead_processs;
	dead_processs = current_process_PCB;

	unlock_scheduler();
	schedule();

	abort("process cleanup process returned: no other processes ready to run. ie: you fucked up");
}

internal void process_start_up(void)
{
	__asm__ volatile("sti");
}
