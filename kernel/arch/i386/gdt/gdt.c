#include "kernel/utils.h"
#include <kernel/gdt.h>
#include <kernel/misc.h>

#include <stdio.h>
#include <string.h>

extern void _gdt_flush(uint32_t);
extern void _tss_flush(uint32_t);

global_variable gdt_entry_t gdt_entries[GDT_SEGMENT_COUNT];
global_variable gdtr_t gdt_ptr;
global_variable struct tss_entry tss_entry;

void gdt_init(void)
{
#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER("init gdt");
#endif

	gdt_ptr.limit = sizeof(gdt_entries) - 1;
	gdt_ptr.base = (uint32_t)&gdt_entries;

	gdt_gate_set(GDT_SEGMENT_NULL, 0, 0, 0, 0);

	gdt_gate_set(GDT_SEGMENT_KCODE, 0, 0XFFFFFFFF, GDT_ACCESS_KCODE,
		     GDT_FLAGS_MAX_4G);

	gdt_gate_set(GDT_SEGMENT_KDATA, 0, 0XFFFFFFFF, GDT_ACCESS_KDATA,
		     GDT_FLAGS_MAX_4G);

	gdt_gate_set(GDT_SEGMENT_UCODE, 0, 0XFFFFFFFF, GDT_ACCESS_UCODE,
		     GDT_FLAGS_MAX_4G);

	gdt_gate_set(GDT_SEGMENT_UDATA, 0, 0XFFFFFFFF, GDT_ACCESS_UDATA,
		     GDT_FLAGS_MAX_4G);

	tss_write(GDT_SEGMENT_TSS, GDT_SEL_KDATA, 0x0);

	_gdt_flush((uint32_t)&gdt_ptr);

	_tss_flush((uint32_t)&tss_entry);

#ifdef DEBUG_LOGGING
	KERNEL_DEBUG_LOGGER("init gdt OK");
#endif
}

void gdt_gate_set(uint32_t num, uint32_t base, uint32_t limit, uint8_t access,
		  uint8_t gran)
{
	gdt_entries[num].access = access;

	gdt_entries[num].base_low = (base & 0xFFFF);
	gdt_entries[num].base_middle = (base >> 16) & 0xFF;
	gdt_entries[num].base_high = (base >> 24) & 0xFF;

	gdt_entries[num].limit = (limit & 0xFFFF);
	gdt_entries[num].flags = (limit >> 16) & 0x0F;
	gdt_entries[num].flags |= (gran & 0xF0);
}

void tss_write(uint32_t num, uint16_t ss0, uint32_t esp0)
{
	uint32_t base = (uint32_t)&tss_entry;
	uint32_t limit = sizeof(tss_entry) - 1;

	gdt_gate_set(num, base, limit, 0xE9, 0x0);

	memset(&tss_entry, 0, sizeof(tss_entry));

	tss_entry.ss0 = ss0;
	tss_entry.esp0 = esp0;

	tss_entry.cs = GDT_OFFSET_KERNEL_CODE | RPL_USER_RING3;
	tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs =
		tss_entry.gs = GDT_SEL_KDATA | RPL_USER_RING3;
}
