#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/misc.h>
#include <kernel/timer.h>
#include <kernel/utils.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

global_variable idt_entry_t idt_entries[IDT_MAX_DESCRIPTORS];
global_variable idtr_t idt_ptr;
void (*irq_routines[16])(struct registers_t *) = { 0 };

extern void _idt_flush(uint32_t);

const char *exception_messages[] = { "division by zero",
				     "debug",
				     "non maskable interrupt",
				     "breakpoint",
				     "into detected overflow",
				     "out of bounds",
				     "invalid opcode",
				     "no coprocessor",
				     "double fault",
				     "coprocessor segment overrun",
				     "bad tss",
				     "segment not present",
				     "stack fault",
				     "general protection fault",
				     "page fault",
				     "unknown interrupt",
				     "coprocessor fault",
				     "alignment fault",
				     "machine check",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved",
				     "reserved" };

void irq_handlers_init(void);
internal void idt_gate_set(uint8_t num, uint32_t base, uint16_t sel,
			   uint8_t flags);

void idt_init(void)
{
#ifdef DEBUG
	KERNEL_DEBUG_LOGGER("init idt");
#endif

	idt_ptr.limit = sizeof(idt_entries) - 1;
	idt_ptr.base = (uint32_t)&idt_entries;

	memset(idt_entries, 0, sizeof(idt_entries));

	// initialize PICS
	outb(PIC1, ICW1_DEFAULT);
	outb(PIC2, ICW1_DEFAULT);

	// set interrupt vector offsets
	outb(PIC1_DATA, PIC1_VECTOR_OFFSET);
	outb(PIC2_DATA, PIC2_VECTOR_OFFSET);

	// set cascade/chaining
	outb(PIC1_DATA, ICW3_SLAVE_ON_IRQ2);
	outb(PIC2_DATA, ICW3_IS_SLAVE);

	// set mode
	outb(PIC1_DATA, ICW4_8086_MODE);
	outb(PIC2_DATA, ICW4_8086_MODE);

	// unmask all interrupts
	outb(PIC1_DATA, PIC_IMR_UNMASK_ALL);
	outb(PIC2_DATA, PIC_IMR_UNMASK_ALL);

	idt_gate_set(0, (uint32_t)isr0, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(1, (uint32_t)isr1, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(2, (uint32_t)isr2, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(3, (uint32_t)isr3, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(4, (uint32_t)isr4, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(5, (uint32_t)isr5, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(6, (uint32_t)isr6, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(7, (uint32_t)isr7, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(8, (uint32_t)isr8, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(9, (uint32_t)isr9, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(10, (uint32_t)isr10, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(11, (uint32_t)isr11, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(12, (uint32_t)isr12, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(13, (uint32_t)isr13, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(14, (uint32_t)isr14, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(15, (uint32_t)isr15, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(16, (uint32_t)isr16, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(17, (uint32_t)isr17, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(18, (uint32_t)isr18, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(19, (uint32_t)isr19, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(20, (uint32_t)isr20, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(21, (uint32_t)isr21, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(22, (uint32_t)isr22, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(23, (uint32_t)isr23, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(24, (uint32_t)isr24, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(25, (uint32_t)isr25, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(26, (uint32_t)isr26, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(27, (uint32_t)isr27, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(28, (uint32_t)isr28, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(29, (uint32_t)isr29, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(30, (uint32_t)isr30, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(31, (uint32_t)isr31, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);

	// interrupt requests
	idt_gate_set(32, (uint32_t)irq0, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(33, (uint32_t)irq1, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(34, (uint32_t)irq2, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(35, (uint32_t)irq3, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(36, (uint32_t)irq4, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(37, (uint32_t)irq5, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(38, (uint32_t)irq6, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(39, (uint32_t)irq7, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(40, (uint32_t)irq8, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(41, (uint32_t)irq9, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(42, (uint32_t)irq10, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(43, (uint32_t)irq11, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(44, (uint32_t)irq12, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(45, (uint32_t)irq13, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(46, (uint32_t)irq14, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(47, (uint32_t)irq15, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);

	// system calls
	idt_gate_set(128, (uint32_t)isr128, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);
	idt_gate_set(177, (uint32_t)isr177, GDT_OFFSET_KERNEL_CODE,
		     IDT_FLAG_KERNEL_INTERRUPT_GATE);

	uint8_t control_byte = PIT_SC_CHANNEL0 | PIT_RW_LOBYTE_ONLY |
			       PIT_MODE_3_SQ_WAVE | PIT_BIN_MODE;
	outb(PIT_CMD_REG_PORT, control_byte);
	outb(PIT_CH0_DATA_PORT, 0x0); // sending 0 treated as 65536
	outb(PIT_CH0_DATA_PORT, 0x0); // sending 0 treated as 65536

	_idt_flush((uint32_t)&idt_ptr);

#ifdef DEBUG
	KERNEL_DEBUG_LOGGER("init idt OK");
#endif
}

internal void idt_gate_set(uint8_t num, uint32_t base, uint16_t sel,
			   uint8_t flags)
{
	idt_entries[num].base_low = base & 0xFFFF;
	idt_entries[num].base_high = (base >> 16) & 0xFFFF;
	idt_entries[num].sel = sel;
	idt_entries[num].always0 = 0;
	idt_entries[num].flags = flags | IDT_FLAG_USER_ACCESS;
}

void irq_install_handler(int irq, void (*handler)(struct registers_t *r))
{
	irq_routines[irq] = handler;
}

void irq_uninstall_handler(int irq)
{
	irq_routines[irq] = 0;
}

void isr_handler(struct registers_t *regs)
{
	if (regs->int_no < 32) {
		abort(exception_messages[regs->int_no]);
	}
}

void irq_handler(struct registers_t *regs)
{
	void (*handler)(struct registers_t *regs);
	handler = irq_routines[regs->int_no - 32];

	if (handler) {
		handler(regs);
	}

	if (regs->int_no >= 40) {
		outb(PIC2, PIC_EOI);
	}

	outb(PIC1, PIC_EOI);
}
