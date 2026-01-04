#include <kernel/idt.h>
#include <kernel/misc.h>
#include <kernel/task.h>
#include <kernel/timer.h>
#include <kernel/tty.h>
#include <kernel/utils.h>

volatile uint64_t ticks_since_boot;
global_variable const uint32_t freq = 1000;

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

void irq_0_handler(unused struct Registers *regs)
{
	ticks_since_boot++;
}

uint64_t read_ticks_since_boot(void)
{
	return ticks_since_boot;
}
