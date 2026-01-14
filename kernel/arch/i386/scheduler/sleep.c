#include <kernel/interrupts/timer.h>
#include <kernel/memory_manager/heap.h>
#include <kernel/memory_manager/vmm.h>
#include <kernel/misc.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/scheduler/sleep.h>
#include <kernel/scheduler/task.h>

#include <stdint.h>

volatile struct TaskControlBlock *sleeping_taskes = NULL;

void micro_sleep_until(uint32_t micro_seconds)
{
	lock_scheduler();

	current_task_PCB->target_wakeup_time = micro_seconds;

	current_task_PCB->next = sleeping_taskes;
	sleeping_taskes = current_task_PCB;

	unlock_scheduler();
	block_task(TASK_STATE_SLEEPING);
}

void micro_sleep(uint32_t micro_seconds)
{
	micro_sleep_until(read_ticks_since_boot() + micro_seconds);
}

void sleep(uint32_t seconds)
{
	micro_sleep(seconds * 1000);
}

void internal_update_awoken_taskes(unused struct Registers *regs,
				   uint64_t ticks_since_boot)
{
	lock_scheduler();

	volatile struct TaskControlBlock *curr = sleeping_taskes;
	volatile struct TaskControlBlock *prev = NULL;

	while (curr) {
		volatile struct TaskControlBlock *next = curr->next;

		if (curr->target_wakeup_time <= ticks_since_boot) {
			if (prev == NULL) {
				sleeping_taskes = next;
			}
			else {
				prev->next = next;
			}

			unblock_task(curr);

			curr = next;
		}
		else {
			prev = curr;
			curr = next;
		}
	}

	unlock_scheduler();
}
