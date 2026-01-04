#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#define TASK_STATE_WAITING	0
#define TASK_STATE_READY_TO_RUN 1
#define TASK_STATE_RUNNING	2

#define TCB_OFFSET_ESP	 0
#define TCB_OFFSET_ESP0	 4
#define TCB_OFFSET_CR3	 8
#define TCB_OFFSET_STATE 16

#ifndef __ASSEMBLER__

#include <kernel/utils.h>
#include <stdint.h>
// current task, task control block
extern volatile struct TaskControlBlock *current_task_TCB;

struct TaskControlBlock {
	uint32_t esp;
	uint32_t esp0;
	uint32_t cr3;
	volatile struct TaskControlBlock *next;
	uint8_t state;
	uint64_t time_used;
} __attribute__((packed));

void multitasking_initialize(void);

void schedule(void);

void create_kernel_task(void (*func)(void));

extern void _switch_to_task(volatile struct TaskControlBlock *next);

uint32_t _forge_kernel_stack(uint32_t stack_top, void (*task)(void),
			     void (*wrapper)(void));

// returns cpu time used in micro second (10^-6s)
uint64_t get_current_task_time_used(void);

#endif // !__ASSEMBLER__

#endif // !_KERNEL_TASK_H
