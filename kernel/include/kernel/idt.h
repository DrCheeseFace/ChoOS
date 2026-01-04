#ifndef _KERNEL_IDT_H
#define _KERNEL_IDT_H

#include <kernel/utils.h>
#include <stdint.h>

#define IDT_MAX_DESCRIPTORS 256

#define IDT_GATE_TYPE_INTERRUPT_32BIT 0x0E
#define IDT_DPL_KERNEL		      0x00
#define IDT_FLAG_PRESENT	      0x80

// 0x8E
#define IDT_FLAG_KERNEL_INTERRUPT_GATE                                         \
	(IDT_FLAG_PRESENT | IDT_DPL_KERNEL | IDT_GATE_TYPE_INTERRUPT_32BIT)

#define IDT_DPL_USER	     0x60
#define IDT_FLAG_USER_ACCESS IDT_DPL_USER

#define PIC1	     0x20 /* IO base address for master PIC */
#define PIC2	     0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA    (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA    (PIC2 + 1)

#define PIC_EOI 0x20

#define ICW1_INIT	    0x10 // initialization sequence starts
#define ICW1_EDGE_TRIGGERED 0x01 // edge triggered mode
#define ICW1_DEFAULT	    (ICW1_INIT | ICW1_EDGE_TRIGGERED) // 0x11

#define PIC1_VECTOR_OFFSET 0x20 // master pic starts at interrupt vector 0x20
#define PIC2_VECTOR_OFFSET 0x28 // slave pic starts at interrupt vector 0x28

#define ICW3_SLAVE_ON_IRQ2 0x04 // master: slave is connected to irq 2
#define ICW3_IS_SLAVE	   0x02 // slave: i am a slave

#define ICW4_8086_MODE 0x01 // set to 8086/8088 mode

#define PIC_IMR_UNMASK_ALL 0x00 // unmask all interrupts

#define PIT_CH0_DATA_PORT 0x40
#define PIT_CH1_DATA_PORT 0x41
#define PIT_CH2_DATA_PORT 0x42
#define PIT_CMD_REG_PORT  0x43

#define PIT_SC_CHANNEL0 0x00
#define PIT_SC_CHANNEL1 0x40
#define PIT_SC_CHANNEL2 0x80
#define PIT_SC_READBACK 0xC0

#define PIT_RW_LATCH	   0x00
#define PIT_RW_LOBYTE_ONLY 0x10
#define PIT_RW_HIBYTE_ONLY 0x20
#define PIT_RW_LOHI_BYTE   0x30

#define PIT_MODE_0_INT_TERM  0x00
#define PIT_MODE_1_ONE_SHOT  0x02
#define PIT_MODE_2_RATE_GEN  0x04
#define PIT_MODE_3_SQ_WAVE   0x06
#define PIT_MODE_4_SW_STROBE 0x08
#define PIT_MODE_5_HW_STROBE 0x0A

#define PIT_BIN_MODE 0x00
#define PIT_BCD_MODE 0x01

typedef struct {
	uint16_t base_low; // The lower 16 bits of the ISR's address
	uint16_t sel; // The GDT segment selector that the CPU will load into CS before calling the ISR
	uint8_t always0;
	uint8_t flags;
	uint16_t base_high; // The higher 16 bits of the ISR's address
} __attribute__((packed)) IdtEntry;

typedef struct {
	uint16_t limit;
	uint32_t base;
} __attribute__((packed)) IdtRegister;

void idt_init(void);

void isr_handler(struct Registers *regs);

void irq_handler(struct Registers *regs);

void irq_install_handler(int irq, void (*handler)(struct Registers *r));

void irq_uninstall_handler(int irq);

extern void isr0(void);
extern void isr1(void);
extern void isr2(void);
extern void isr3(void);
extern void isr4(void);
extern void isr5(void);
extern void isr6(void);
extern void isr7(void);
extern void isr8(void);
extern void isr9(void);
extern void isr10(void);
extern void isr11(void);
extern void isr12(void);
extern void isr13(void);
extern void isr14(void);
extern void isr15(void);
extern void isr16(void);
extern void isr17(void);
extern void isr18(void);
extern void isr19(void);
extern void isr20(void);
extern void isr21(void);
extern void isr22(void);
extern void isr23(void);
extern void isr24(void);
extern void isr25(void);
extern void isr26(void);
extern void isr27(void);
extern void isr28(void);
extern void isr29(void);
extern void isr30(void);
extern void isr31(void);

extern void irq0(void);
extern void irq1(void);
extern void irq2(void);
extern void irq3(void);
extern void irq4(void);
extern void irq5(void);
extern void irq6(void);
extern void irq7(void);
extern void irq8(void);
extern void irq9(void);
extern void irq10(void);
extern void irq11(void);
extern void irq12(void);
extern void irq13(void);
extern void irq14(void);
extern void irq15(void);

extern void isr128(void);
extern void isr177(void);

#endif // ! _KERNEL_IDT_H
