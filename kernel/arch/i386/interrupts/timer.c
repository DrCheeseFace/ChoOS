#include <kernel/heap.h>
#include <kernel/idt.h>
#include <kernel/misc.h>
#include <kernel/process.h>
#include <kernel/timer.h>
#include <kernel/tty.h>
#include <kernel/utils.h>

volatile uint64_t ticks_since_boot;
global_variable const uint32_t freq = 1000;

// arr of installed irq0 timer funcs
struct TimerEvent {
	void (*func)(struct Registers *regs, uint64_t ticks_since_boot);
	struct TimerEvent *next;
};

global_variable struct TimerEvent *timer_events = NULL;

void irq_0_handler(unused struct Registers *regs);

void timer_init(void)
{
	KERNEL_DEBUG_LOGGER("initing timer");
	ticks_since_boot = 0;
	irq_install_handler(0, &irq_0_handler);
	uint32_t divisor = FREQ_HZ / freq;

	uint8_t command_byte = PIT_SC_CHANNEL0 | PIT_RW_LOHI_BYTE |
			       PIT_MODE_3_SQ_WAVE | PIT_BIN_MODE;

	outb(PIT_CMD_REG_PORT, command_byte);
	outb(PIT_CH0_DATA_PORT, (uint8_t)divisor & 0xFF);
	outb(PIT_CH0_DATA_PORT, (uint8_t)(divisor >> 8) & 0xFF);

	KERNEL_DEBUG_LOGGER("init timer OK");
}

TimerEvent *install_timer_event(void (*func)(struct Registers *regs,
					     uint64_t ticks_since_boot))
{
	struct TimerEvent *new_event = kmalloc(sizeof(*new_event));
	if (!new_event) {
		return NULL;
	}

	new_event->func = func;
	new_event->next = NULL;

	if (!timer_events) {
		timer_events = new_event;
	}
	else {
		new_event->next = timer_events;
		timer_events = new_event;
	}

	return new_event;
}

void uninstall_timer_event(struct TimerEvent *event)
{
	lock_scheduler();
	if (event == NULL || timer_events == NULL) {
		goto exit;
	}

	if (timer_events == event) {
		timer_events = event->next;
		kfree(event);
		goto exit;
	}

	struct TimerEvent *current = timer_events;
	while (current->next != NULL) {
		if (current->next == event) {
			current->next = event->next;
			kfree(event);
			goto exit;
		}
		current = current->next;
	}

exit:
	unlock_scheduler();
	return;
}

void irq_0_handler(unused struct Registers *regs)
{
	ticks_since_boot++;

	struct TimerEvent *node = timer_events;
	while (node) {
		node->func(regs, ticks_since_boot);
		node = node->next;
	}
}

uint64_t read_ticks_since_boot(void)
{
	return ticks_since_boot;
}
