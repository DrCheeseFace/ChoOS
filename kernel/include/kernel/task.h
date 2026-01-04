#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#include <kernel/utils.h>
#include <stdint.h>

// current processes, process control block
extern struct ProcessControlBlock *current_process_PCB;

struct ProcessControlBlock {
	uint32_t esp;
	uint32_t esp0;
	uint32_t cr3;
	struct ProcessControlBlock *next;
	uint8_t state;
} __attribute__((packed));

void multitasking_initialize(void);

void schedule(void);

void create_kernel_task(void (*func)(void));

extern void _switch_to_task(struct ProcessControlBlock *next);

uint32_t _forge_kernel_stack(uint32_t stack_top, void (*task)(void),
			     void (*wrapper)(void));

#endif // !_KERNEL_TASK_H
