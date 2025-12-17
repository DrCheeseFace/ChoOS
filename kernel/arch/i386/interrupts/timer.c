#include "kernel/utils.h"
#include <kernel/idt.h>
#include <kernel/misc.h>
#include <kernel/timer.h>
#include <kernel/tty.h>

global_variable uint64_t ticks;
global_variable const uint32_t freq = 1000;

void irq_0_handler(unused struct interrupt_resigters *regs);

void timer_init(void)
{
#ifdef DEBUG
	KERNEL_DEBUG_LOGGER("initing timer");
#endif
	ticks = 0;
	irq_install_handler(0, &irq_0_handler);
	uint32_t divisor = FREQ_HZ / freq;

	uint8_t command_byte = PIT_SC_CHANNEL0 | PIT_RW_LOHI_BYTE |
			       PIT_MODE_3_SQ_WAVE | PIT_BIN_MODE;

	outb(PIT_CMD_REG_PORT, command_byte);
	outb(PIT_CH0_DATA_PORT, (uint8_t)divisor & 0xFF);
	outb(PIT_CH0_DATA_PORT, (uint8_t)(divisor >> 8) & 0xFF);

#ifdef DEBUG
	KERNEL_DEBUG_LOGGER("init timer OK");
#endif
}

void irq_0_handler(unused struct interrupt_resigters *regs)
{
	ticks++;
}
