
#ifndef _KERNEL_TASK_INTERNAL_H
#define _KERNEL_TASK_INTERNAL_H

#include <kernel/misc.h>
#include <kernel/utils.h>
#include <stdint.h>

void internal_update_awoken_taskes(unused struct Registers *regs,
				   uint64_t ticks_since_boot);

#endif
