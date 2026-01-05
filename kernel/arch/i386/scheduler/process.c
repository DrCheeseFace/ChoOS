#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/heap_internal.h>
#include <kernel/idt.h>
#include <kernel/paging.h>
#include <kernel/process.h>
#include <kernel/timer.h>
#include <kernel/utils.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

global_variable uint64_t last_tick;
global_variable uint64_t current_pid = 0;

volatile struct ProcessControlBlock *current_process_PCB = NULL;
volatile struct ProcessControlBlock *first_ready_to_run_process = NULL;
volatile struct ProcessControlBlock *last_ready_to_run_process = NULL;

volatile struct ProcessControlBlock *dead_processs = NULL;

void process_start_up(void);
internal void update_time_used(void);

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

	__asm__ volatile("sti");

	KERNEL_DEBUG_LOGGER("init multiprocessing OK");
}

internal void free_process(volatile struct ProcessControlBlock *node)
{
	kfree((void *)(node->stack_base));
	kfree((void *)node);
}

void dead_processs_cleanup(void)
{
	__asm__ volatile("cli");
	KERNEL_DEBUG_LOGGER("dead_processs_cleanup entry");
	internal_heap_log_info();

	volatile struct ProcessControlBlock *node = dead_processs;
	while (node) {
		volatile struct ProcessControlBlock *next = node->next;
		free_process(node);
		node = next;
	}
	dead_processs = NULL;

	internal_heap_log_info();

	KERNEL_DEBUG_LOGGER("dead_processs_cleanup exit");
	__asm__ volatile("sti");
}

void mark_process_dead(void)
{
	printf("currentid %d\n", current_process_PCB->id);
	current_process_PCB->state = PROCESS_STATE_DEAD;
	current_process_PCB->next = NULL;
	if (dead_processs) {
		dead_processs->next = current_process_PCB;
	}
	else {
		dead_processs = current_process_PCB;
	}

	schedule();

	abort("process cleanup process returned");
}

uint32_t create_kernel_process(void (*func)(void))
{
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
		stack_top, func, process_start_up, mark_process_dead);
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

	return new_process->id;
}

void schedule(void)
{
	if (!first_ready_to_run_process)
		return;

	volatile struct ProcessControlBlock *process_to_run =
		first_ready_to_run_process;
	first_ready_to_run_process = process_to_run->next;

	update_time_used();

	if (current_process_PCB->state == PROCESS_STATE_RUNNING) {
		current_process_PCB->state = PROCESS_STATE_READY_TO_RUN;

		if (last_ready_to_run_process) {
			last_ready_to_run_process->next = current_process_PCB;
		}
		else {
			first_ready_to_run_process = current_process_PCB;
		}

		last_ready_to_run_process = current_process_PCB;
		last_ready_to_run_process->next = NULL;
	}

	_switch_to_process(process_to_run);
}

void process_start_up(void)
{
	__asm__ volatile("sti");
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
