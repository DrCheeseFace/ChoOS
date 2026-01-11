#ifndef _KERNEL_TIMER_H
#define _KERNEL_TIMER_H

#include <kernel/utils.h>

#define FREQ_HZ 1193180

void timer_init(void);
uint64_t read_ticks_since_boot(void);

typedef void TimerEvent;

// installed func will run every timer tick
// returns handler for obliteration later
// returns NULL if failed
TimerEvent *install_timer_event(void (*func)(struct Registers *regs,
					     uint64_t ticks_since_boot));

#endif // ! _KERNEL_TIMER_H
