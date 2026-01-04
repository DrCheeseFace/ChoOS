#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

#include <kernel/utils.h>

#define FREQ_HZ 1193180

void timer_init(void);
uint64_t read_ticks_since_boot(void);

#endif // ! _KERNEL_TIMER_H
