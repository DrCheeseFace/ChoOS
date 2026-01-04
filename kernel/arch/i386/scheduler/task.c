#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/idt.h>
#include <kernel/paging.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/utils.h>

#include <stdint.h>
#include <string.h>

global_variable uint64_t last_tick;
struct ProcessControlBlock *current_process_PCB;

void task_start_up(void);
internal void update_time_used(void);

void multitasking_initialize(void)
{
	KERNEL_DEBUG_LOGGER("init multitasking");

	__asm__ volatile("cli");

	struct ProcessControlBlock *kernel_process =
		kmalloc(sizeof(*kernel_process));
	memset(kernel_process, 0, sizeof(*kernel_process));

	uint32_t cr3 = _read_cr3();
	kernel_process->cr3 = cr3;
	kernel_process->next = kernel_process;
	current_process_PCB = kernel_process;

	last_tick = read_ticks_since_boot();

	__asm__ volatile("sti");

	KERNEL_DEBUG_LOGGER("init gdt OK");
}

void create_kernel_task(void (*func)(void))
{
	struct ProcessControlBlock *new_process = kmalloc(sizeof(*new_process));
	memset(new_process, 0, sizeof(*new_process));

	void *stack_base = kmalloc(KERNEL_STACK_SIZE);

	uint32_t stack_top = (uint32_t)stack_base + KERNEL_STACK_SIZE;

	new_process->esp = _forge_kernel_stack(stack_top, func, task_start_up);
	new_process->esp0 = (uint32_t)stack_top;
	new_process->cr3 = _read_cr3();

	new_process->next = current_process_PCB->next;
	current_process_PCB->next = new_process;
}

void schedule(void)
{
	if (!current_process_PCB)
		return;

	if (current_process_PCB->next == current_process_PCB) {
		return;
	}

	update_time_used();
	_switch_to_task(current_process_PCB->next);
}

void task_start_up(void)
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
