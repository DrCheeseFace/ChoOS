# figure it out
- (bozo)

<img src="https://i.makeagif.com/media/9-14-2018/f65VKy.gif" width="100%"/>

# TODO
- [x] printing to terminal
- [x] gdt
- [x] idt
- [x] timer interrupts
- [x] keyboard interrupts
- [x] enable paging
- [x] replace nasm with just S cuz why do both lol
- [x] jump to higher half kernel
- [ ] virtual memory management kmalloc, kfree, krealloc, kcalloc
    - [ ] heap implementation
- [ ] move tests over to using mrt_test framework. (after kmalloc implementation)
- [ ] bitmap to some other physical memory allocators
- [ ] can there be multile regions of free memory to allocate page frames for?
- [ ] processes, schedule(), yield() etc

# TODO implementations
paddr_t vmm_virt_to_phys(vaddr_t virt)
    purpose: debugging and dma. drivers often need the physical address of a buffer to give to hardware.
    logic: walk the page tables and return the physical address found in the pte + offset.

void* vmm_create_new_context(void)
    purpose: creates a new, blank page map level 4 (pml4) or page directory.
    logic: allocates a page for the top-level structure. crucially, it must map the kernel
    into this new context (usually the higher half)
    so that interrupts work regardless of which process is running.

void vmm_switch_context(void* pml4_phys_addr)
    purpose: context switching.
    implementation: on x86, this writes the physical address of the new directory into the cr3 register.

void vmm_destroy_context(void* pml4_phys_addr)
    purpose: clean up when a process dies.
    logic: frees the page tables associated with this process (but be careful not to free the kernel's pages!).

mapping one page at a time is tedious. you usually want to map a range.
int vmm_map_range(paddr_t phys_start, vaddr_t virt_start, size_t count, uint32_t flags)
    purpose: loops vmm_map_page count times.
    use case: mapping a framebuffer, mmio (memory mapped i/o), or loading a kernel section.

void vmm_page_fault_handler(registers_t* regs)
    purpose: called by the isr (interrupt service routine) when interrupt 14 (on x86) fires.
    logic:
        read cr2 (on x86) to find the address that caused the fault.
        check error code (was it a permission violation? was the page not present?).
        if valid (e.g., copy-on-write or lazy loading), fix the mapping and return.
        if invalid (e.g., null pointer dereference), terminate the process.
