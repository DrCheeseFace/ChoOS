#include <kernel/heap.h>
#include <kernel/misc.h>
#include <kernel/process.h>
#include <kernel/sleep.h>
#include <kernel/timer.h>

#include <stdint.h>

struct SleepNode {
	volatile struct ProcessControlBlock *proc;
	uint64_t target_wakeup_time;
	struct SleepNode *next;
};

global_variable struct SleepNode *sleeping_processes = NULL;

void nano_sleep_until(uint32_t nano_seconds)
{
	struct SleepNode *new_node = kmalloc(sizeof(*new_node));
	new_node->target_wakeup_time = nano_seconds;
	new_node->next = sleeping_processes;
	new_node->proc = current_process_PCB;
	sleeping_processes = new_node;

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

	struct SleepNode **curr = &sleeping_processes;

	while (*curr) {
		struct SleepNode *entry = *curr;

		if (entry->target_wakeup_time <= ticks_since_boot) {
			unblock_process(entry->proc);
			*curr = entry->next;
			kfree(entry);
		}
		else {
			curr = &entry->next;
		}
	}

	unlock_scheduler();
}
