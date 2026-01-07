#ifndef _KERNEL_SLEEP_H
#define _KERNEL_SLEEP_H

#include <stdint.h>

// sleep until read_ticks_since_boot();
void nano_sleep_until(uint32_t nano_seconds);

void nano_sleep(uint32_t nano_seconds);

void sleep(uint32_t seconds);

#endif
