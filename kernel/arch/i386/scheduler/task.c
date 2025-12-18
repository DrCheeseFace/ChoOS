//
// NOT IMPLEMENTED YET
// NOT IMPLEMENTED YET
// NOT IMPLEMENTED YET
// NOT IMPLEMENTED YET
// NOT IMPLEMENTED YET
// NOT IMPLEMENTED YET
//
#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/paging.h>
#include <kernel/task.h>
#include <kernel/utils.h>
#include <stdint.h>
#include <string.h>

process_t *pqueue;
process_t *current_proc;

void init_process_queue(int32_t kernel_directory_ptr)
{
	__asm__ volatile("cli");

	current_proc = (process_t *)(void *)kmalloc_page();
	memset(current_proc, 0, sizeof(process_t));

	current_proc->cr3 = kernel_directory_ptr;
	current_proc->next = NULL;

	pqueue = current_proc;
}

void switch_task_irq0_handler(struct registers_t *regs)
{
	memcpy(&current_proc->regs, regs, sizeof(struct registers_t));

	if (current_proc->next != NULL)
		current_proc = current_proc->next;
	else
		current_proc = pqueue;

	memcpy(regs, &current_proc->regs, sizeof(struct registers_t));

	_loadPageDirectory(current_proc->cr3);
}
