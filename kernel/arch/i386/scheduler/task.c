#include <kernel/gdt.h>
#include <kernel/heap.h>
#include <kernel/idt.h>
#include <kernel/paging.h>
#include <kernel/task.h>
#include <kernel/utils.h>
#include <stdint.h>
#include <string.h>

struct ProcessControlBlock *current_process_PCB;

void task_start_up(void);

void multitasking_initialize(void)
{
	KERNEL_DEBUG_LOGGER("init multitasking");

	__asm__ volatile("cli");

	struct ProcessControlBlock *kernel_process =
		kmalloc(sizeof(struct ProcessControlBlock));
	memset(kernel_process, 0, sizeof(*kernel_process));

	uint32_t cr3 = _read_cr3();
	kernel_process->cr3 = cr3;
	kernel_process->next = kernel_process;
	current_process_PCB = kernel_process;

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

	struct ProcessControlBlock *next_task = current_process_PCB->next;

	if (next_task == current_process_PCB) {
		return;
	}

	_switch_to_task(next_task);
}

void task_start_up(void)
{
	__asm__ volatile("sti");
}
