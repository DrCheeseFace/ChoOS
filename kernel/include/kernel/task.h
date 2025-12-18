#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <kernel/utils.h>
#include <stdint.h>

struct process {
	struct registers_t regs;
	uint32_t cr3;
	struct process *next;
} __attribute__((packed));

void switch_task_irq0_handler(struct registers_t *regs);

void init_process_queue(int32_t kernel_directory_ptr);

#endif // !_KERNEL_TASK_H
