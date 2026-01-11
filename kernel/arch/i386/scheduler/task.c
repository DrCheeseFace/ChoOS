#include <kernel/gdt.h>
#include <kernel/interrupts/idt.h>
#include <kernel/memory_manager/heap.h>
#include <kernel/memory_manager/heap_internal.h>
#include <kernel/paging/paging.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/scheduler/sleep.h>
#include <kernel/scheduler/task.h>
#include <kernel/utils.h>

#include <stdint.h>
#include <stdlib.h>

int get_task_state(PID pid)
{
	if (current_task_PCB && current_task_PCB->id == pid) {
		return current_task_PCB->state;
	}

	// check ready to run tasks
	struct ProcessControlBlock *node = (void *)first_ready_to_run_task;
	while (node) {
		if (node->id == pid) {
			return node->state;
		}
		node = (void *)node->next;
	}

	// check sleeping tasks
	node = (void *)sleeping_taskes;
	while (node) {
		if (node->id == pid) {
			return node->state;
		}
		node = (void *)node->next;
	}

	// check dead/ terminated tasks
	node = (void *)dead_tasks;
	while (node) {
		if (node->id == pid) {
			return node->state;
		}
		node = (void *)node->next;
	}

	// task doesnt exist or already cleanedup
	return -1;
}

void block_task(int reason)
{
	lock_scheduler();

	current_task_PCB->state = reason;

	// other way aaround?
	unlock_scheduler();
	schedule();
}

void unblock_task(volatile struct ProcessControlBlock *task)
{
	lock_scheduler();

	task->state = TASK_STATE_READY_TO_RUN;
	task->next = NULL;

	if (first_ready_to_run_task == NULL) {
		first_ready_to_run_task = task;
		last_ready_to_run_task = task;
	}
	else {
		last_ready_to_run_task->next = task;
		last_ready_to_run_task = task;
	}

	unlock_scheduler();
}

void terminate_task(void)
{
	lock_scheduler();

	// push to front of dead queue
	current_task_PCB->next = dead_tasks;
	dead_tasks = current_task_PCB;

	unlock_scheduler();

	block_task(TASK_STATE_TERMINATED);

	abort("task terminate_task returned: no other taskes ready to run. ie: you fucked up");
}

uint64_t get_current_task_time_used(void)
{
	update_time_used();
	return current_task_PCB->time_used;
}
