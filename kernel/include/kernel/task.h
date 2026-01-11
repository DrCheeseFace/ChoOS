#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#define TASK_STATE_DEAD		0
#define TASK_STATE_READY_TO_RUN 1
#define TASK_STATE_RUNNING	2
#define TASK_STATE_SLEEPING	3

#define PCB_OFFSET_ID	 0
#define PCB_OFFSET_ESP	 4
#define PCB_OFFSET_ESP0	 8
#define PCB_OFFSET_CR3	 12
#define PCB_OFFSET_STATE 20

#ifndef __ASSEMBLER__

#include <kernel/utils.h>
#include <stdint.h>
// current task, task control block
extern volatile struct ProcessControlBlock *current_task_PCB;
// sleeping taskes
extern volatile struct ProcessControlBlock *sleeping_taskes;
extern volatile uint32_t IRQ_disable_counter;
extern volatile uint32_t postpone_task_switches_counter;
extern volatile uint8_t task_switches_postponed_flag;

typedef uint32_t PID;

struct ProcessControlBlock {
	PID id;
	uint32_t esp;
	uint32_t esp0;
	uint32_t cr3;
	volatile struct ProcessControlBlock *next;
	uint8_t state;
	uint64_t time_used;
	void *stack_base;
	uint64_t target_wakeup_time;
} __attribute__((packed));

void multitasking_initialize(void);

void schedule(void);
void lock_scheduler(void);
void unlock_scheduler(void);

// returns pid
uint32_t create_kernel_task(void (*func)(void));

// Returns
//     TASK_STATE_DEAD	   0
//     TASK_STATE_READY_TO_RUN  1
//     TASK_STATE_RUNNING	   2
//     TASK_STATE_SLEEPING	   3
// Returns -1 if not found
int get_task_state(PID pid);

// Args: reason
//     TASK_STATE_DEAD	   0
//     TASK_STATE_READY_TO_RUN  1
//     TASK_STATE_RUNNING	   2
//     TASK_STATE_SLEEPING	   3
void block_task(int reason);
void unblock_task(volatile struct ProcessControlBlock *task);

extern void _switch_to_task(volatile struct ProcessControlBlock *next);

uint32_t _forge_kernel_stack(uint32_t stack_top, void (*task)(void),
			     void (*task_start_up)(void),
			     void (*cleanup_func)(void));

// returns cpu time used in micro second (10^-6s)
uint64_t get_current_task_time_used(void);

void dead_tasks_cleanup(void);

#endif // !__ASSEMBLER__

#endif // !_KERNEL_TASK_H
