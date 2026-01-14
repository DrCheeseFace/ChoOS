#ifndef _KERNEL_SCHEDULER_H
#define _KERNEL_SCHEDULER_H

#define PCB_OFFSET_ID	 0
#define PCB_OFFSET_ESP	 4
#define PCB_OFFSET_ESP0	 8
#define PCB_OFFSET_CR3	 12
#define PCB_OFFSET_STATE 20

#ifndef __ASSEMBLER__

#include <kernel/scheduler/task.h>
#include <stdint.h>

// current task, task control block
extern volatile struct TaskControlBlock *current_task_PCB;
extern volatile struct TaskControlBlock *first_ready_to_run_task;
extern volatile struct TaskControlBlock *last_ready_to_run_task;
extern volatile struct TaskControlBlock *dead_tasks;

struct TaskControlBlock {
	PID id;
	uint32_t esp;
	uint32_t esp0;
	uint32_t cr3;
	volatile struct TaskControlBlock *next;
	int state;
	uint64_t time_used;
	void *stack_base;
	uint64_t target_wakeup_time;
} __attribute__((packed));

void multitasking_initialize(void);

struct TaskControlBlock *create_kernel_task(void (*func)(void));

void schedule(void);
void lock_scheduler(void);
void unlock_scheduler(void);

void update_time_used(void);
void dead_tasks_cleanup(void);

#endif // !__ASSEMBLER__
#endif // !_KERNEL_SCHEDULER_H
