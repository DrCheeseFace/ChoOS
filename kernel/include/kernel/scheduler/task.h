#ifndef _KERNEL_TASK_H
#define _KERNEL_TASK_H

#define TASK_STATE_TERMINATED	-2
#define TASK_STATE_NOT_FOUND	-1
#define TASK_STATE_DEAD		0
#define TASK_STATE_READY_TO_RUN 1
#define TASK_STATE_RUNNING	2
#define TASK_STATE_SLEEPING	3

#ifndef __ASSEMBLER__

#include <kernel/utils.h>
#include <stdint.h>

typedef uint32_t PID;
struct ProcessControlBlock;

/*
 * Returns
 *    TASK_STATE_TERMINATED	-2
 *    TASK_STATE_NOT_FOUND	-1
 *    TASK_STATE_DEAD		0
 *    TASK_STATE_READY_TO_RUN   1
 *    TASK_STATE_RUNNING        2
 *    TASK_STATE_SLEEPING	3
 */
int get_task_state(PID pid);

/*
 * Args: reason
 *     TASK_STATE_DEAD	           0
 *     TASK_STATE_READY_TO_RUN     1
 *     TASK_STATE_RUNNING	   2
 *     TASK_STATE_SLEEPING	   3
 */
void block_task(int reason);
void unblock_task(volatile struct ProcessControlBlock *task);

void terminate_task(void);

// returns cpu time used in micro second (10^-6s)
uint64_t get_current_task_time_used(void);

#endif // !__ASSEMBLER__

#endif // !_KERNEL_TASK_H
