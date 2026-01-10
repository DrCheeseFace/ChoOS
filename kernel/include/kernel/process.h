#ifndef _KERNEL_PROCESS_H
#define _KERNEL_PROCESS_H

#define PROCESS_STATE_DEAD	   0
#define PROCESS_STATE_READY_TO_RUN 1
#define PROCESS_STATE_RUNNING	   2
#define PROCESS_STATE_SLEEPING	   3

#define PCB_OFFSET_ID	 0
#define PCB_OFFSET_ESP	 4
#define PCB_OFFSET_ESP0	 8
#define PCB_OFFSET_CR3	 12
#define PCB_OFFSET_STATE 20

#ifndef __ASSEMBLER__

#include <kernel/utils.h>
#include <stdint.h>
// current process, process control block
extern volatile struct ProcessControlBlock *current_process_PCB;
// sleeping processes
extern volatile struct ProcessControlBlock *sleeping_processes;
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

void multiprocessing_initialize(void);

void schedule(void);
void lock_scheduler(void);
void unlock_scheduler(void);

// returns pid
uint32_t create_kernel_process(void (*func)(void));

// Returns
//     PROCESS_STATE_DEAD	   0
//     PROCESS_STATE_READY_TO_RUN  1
//     PROCESS_STATE_RUNNING	   2
//     PROCESS_STATE_SLEEPING	   3
// Returns -1 if not found
int get_process_state(PID pid);

// Args: reason
//     PROCESS_STATE_DEAD	   0
//     PROCESS_STATE_READY_TO_RUN  1
//     PROCESS_STATE_RUNNING	   2
//     PROCESS_STATE_SLEEPING	   3
void block_process(int reason);
void unblock_process(volatile struct ProcessControlBlock *process);

extern void _switch_to_process(volatile struct ProcessControlBlock *next);

uint32_t _forge_kernel_stack(uint32_t stack_top, void (*process)(void),
			     void (*process_start_up)(void),
			     void (*cleanup_func)(void));

// returns cpu time used in micro second (10^-6s)
uint64_t get_current_process_time_used(void);

void dead_processs_cleanup(void);

#endif // !__ASSEMBLER__

#endif // !_KERNEL_PROCESS_H
