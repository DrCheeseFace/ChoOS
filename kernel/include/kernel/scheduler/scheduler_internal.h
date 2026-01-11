
#ifndef _KERNEL_TASK_INTERNAL_H
#define _KERNEL_TASK_INTERNAL_H

#include <kernel/misc.h>
#include <kernel/scheduler/scheduler.h>
#include <kernel/utils.h>

#include <stdint.h>

void internal_update_awoken_taskes(unused struct Registers *regs,
				   uint64_t ticks_since_boot);

extern void _switch_to_task(volatile struct ProcessControlBlock *next);

uint32_t _forge_kernel_stack(uint32_t stack_top, void (*task)(void),
			     void (*task_start_up)(void),
			     void (*cleanup_func)(void));

#endif
