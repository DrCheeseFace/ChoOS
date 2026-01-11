#ifndef _KERNEL_SLEEP_H
#define _KERNEL_SLEEP_H

#include <stdint.h>

// sleeping tasks
extern volatile struct ProcessControlBlock *sleeping_taskes;

// sleep until read_ticks_since_boot();
void micro_sleep_until(uint32_t micro_seconds);

void micro_sleep(uint32_t micro_seconds);

void sleep(uint32_t seconds);

#endif
