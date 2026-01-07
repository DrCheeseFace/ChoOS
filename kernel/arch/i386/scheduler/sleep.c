#include <kernel/heap.h>
#include <kernel/misc.h>
#include <kernel/process.h>
#include <kernel/sleep.h>
#include <kernel/timer.h>

#include <stdint.h>

void nano_sleep_until(uint32_t nano_seconds)
{
	current_process_PCB->target_wakeup_time = nano_seconds;
	block_process(PROCESS_STATE_SLEEPING);
}

void nano_sleep(uint32_t nano_seconds)
{
	nano_sleep_until(read_ticks_since_boot() + nano_seconds);
}

void sleep(uint32_t seconds)
{
	nano_sleep(seconds * 1000);
}

void internal_update_awoken_processes(unused struct Registers *regs,
				      uint64_t ticks_since_boot)
{
	lock_scheduler();

	volatile struct ProcessControlBlock *curr = sleeping_processes;

	while (curr) {
		volatile struct ProcessControlBlock *next = curr->next;

		if (curr->target_wakeup_time <= ticks_since_boot) {
			unblock_process(curr);
		}

		curr = next;
	}

	unlock_scheduler();
}
