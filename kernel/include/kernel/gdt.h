#ifndef _KERNEL_GDT_H
#define _KERNEL_GDT_H

#include <stdint.h>

#define GDT_OFFSET_KERNEL_CODE 0x08

enum GdtSegmentType {
	GDT_SEGMENT_NULL = 0,
	GDT_SEGMENT_KCODE,
	GDT_SEGMENT_KDATA,
	GDT_SEGMENT_UCODE,
	GDT_SEGMENT_UDATA,
	GDT_SEGMENT_TSS,
	GDT_SEGMENT_COUNT
};

#define GDT_ACCESS_KCODE 0x9A
#define GDT_ACCESS_KDATA 0x92
#define GDT_ACCESS_UCODE 0xFA
#define GDT_ACCESS_UDATA 0xF2

#define GDT_FLAGS_MAX_4G 0xCF

#define GDT_SEL_KDATA 0x10
// requested privilege level for user mode (ring 3)
#define RPL_USER_RING3 0x3

// tss segment selectors (selector | rpl)
#define TSS_SEL_KCODE (GDT_OFFSET_KERNEL_CODE | RPL_USER_RING3)
#define TSS_SEL_UDATA (GDT_SEL_KDATA | RPL_USER_RING3)

typedef struct {
	uint16_t limit;
	uint16_t base_low;
	uint8_t base_middle;
	uint8_t access;
	uint8_t flags;
	uint8_t base_high;
} __attribute__((packed)) GdtEntry;

typedef struct {
	uint16_t limit;
	uintptr_t base;
} __attribute__((packed)) GdtRegister;

extern volatile struct TssEntry tss_entry;
struct TssEntry {
	uint32_t prev_tss;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint32_t trap;
	uint32_t iomap_base;
} __attribute__((packed));

void gdt_init(void);

void gdt_gate_set(uint32_t num, uint32_t base, uint32_t limit, uint8_t access,
		  uint8_t gran);

void tss_write(uint32_t num, uint16_t ss0, uint32_t esp0);

#endif // !
