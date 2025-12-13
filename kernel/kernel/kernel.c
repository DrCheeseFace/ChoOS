#include <kernel/gdt.h>
#include <kernel/idt.h>
#include <kernel/keyboard.h>
#include <kernel/multiboot.h>
#include <kernel/paging.h>
#include <kernel/ssp.h>
#include <kernel/timer.h>
#include <kernel/tty.h>

void kernel_main(uint32_t magic, multiboot_info_t *mbd)
{
	tty_init();
	paging_init(magic, mbd);
	gdt_init();
	idt_init();
	timer_init();
	keyboard_init();

	for (;;)
		;
}
