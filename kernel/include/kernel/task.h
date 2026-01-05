#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#define TASK_STATE_WAITING	0
#define TASK_STATE_READY_TO_RUN 1
#define TASK_STATE_RUNNING	2
#define TASK_STATE_DEAD		3

#define TCB_OFFSET_ID	 0
#define TCB_OFFSET_ESP	 4
#define TCB_OFFSET_ESP0	 8
#define TCB_OFFSET_CR3	 12
#define TCB_OFFSET_STATE 20

#ifndef __ASSEMBLER__

#include <kernel/utils.h>
#include <stdint.h>
// current task, task control block
extern volatile struct TaskControlBlock *current_task_TCB;

struct TaskControlBlock {
	uint32_t id;
	uint32_t esp;
	uint32_t esp0;
	uint32_t cr3;
	volatile struct TaskControlBlock *next;
	uint8_t state;
	uint64_t time_used;
	void *stack_base;
} __attribute__((packed));

void multitasking_initialize(void);

void schedule(void);

// returns pid
uint32_t create_kernel_task(void (*func)(void));

extern void _switch_to_task(volatile struct TaskControlBlock *next);

uint32_t _forge_kernel_stack(uint32_t stack_top, void (*task)(void),
			     void (*task_start_up)(void),
			     void (*cleanup_func)(void));

// returns cpu time used in micro second (10^-6s)
uint64_t get_current_task_time_used(void);

void dead_tasks_cleanup(void);

#endif // !__ASSEMBLER__

#endif // !_KERNEL_TASK_H
