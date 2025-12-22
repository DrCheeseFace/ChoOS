# Declare constants for the multiboot header.
.set ALIGN,    1<<0             # align loaded modules on page boundaries
.set MEMINFO,  1<<1             # provide memory map
.set FLAGS,    ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,    0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM, -(MAGIC + FLAGS) # checksum of above, to prove we are multiboot

# Declare a multiboot header that marks the program as a kernel.
.section .multiboot.data, "aw"
.align 4
.long MAGIC
.long FLAGS
.long CHECKSUM

# Allocate the initial stack.
.section .bootstrap_stack, "aw", @nobits
stack_bottom:
.skip 16384 # 16 KiB
stack_top:

# Preallocate pages used for paging. Don't hard-code addresses and assume they
# are available, as the bootloader might have loaded its multiboot structures or
# modules there. This lets the bootloader know it must avoid the addresses.
.section .bss, "aw", @nobits
	.align 4096
boot_page_directory:
	.skip 4096
boot_page_table1:
	.skip 4096
# Further page tables may be required if the kernel grows beyond 3 MiB.


# The kernel entry point.
.section .multiboot.text, "a"
.global _start
.type _start, @function
_start:
    movl $(boot_page_table1 - 0xC0000000), %edi
    movl $0, %esi
    movl $1023, %ecx

1:
    # map physical address to page table entry
    movl %esi, %edx
    orl $0x003, %edx
    movl %edx, (%edi)

    addl $4096, %esi
    addl $4, %edi
    loop 1b

    # map vga video memory to 0xc03ff000
    ; movl $(0x000B8000 | 0x003), boot_page_table1 - 0xC0000000 + 1023 * 4

    # Identity Map (Entry 0)
    movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 0
    # Higher Half Map (Entry 768)
    movl $(boot_page_table1 - 0xC0000000 + 0x003), boot_page_directory - 0xC0000000 + 768 * 4

    # load cr3
    movl $(boot_page_directory - 0xC0000000), %ecx
    movl %ecx, %cr3

    # enable paging
    movl %cr0, %ecx
    orl $0x80010000, %ecx
    movl %ecx, %cr0

    # jump to higher half
    lea 4f, %ecx
    jmp *%ecx

.section .text

4:
    mov $stack_top, %esp

    addl $0xC0000000, %ebx

    pushl %ebx
    pushl %eax

    call kernel_main

    cli
1:  hlt
    jmp 1b
