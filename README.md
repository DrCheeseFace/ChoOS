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
- [x] virtual memory management kmalloc, kfree, krealloc, kcalloc
    - [x] heap implementation
    - [x] all heap blocks be aligned to 8/16bits? am i bothered? google firstfit vs bestfit
    - [ ] add tests for kmalloc kfree
- [ ] move tests over to using mrt_test framework. (after malloc implementation)
- [ ] bitmap to some other physical memory allocators
- [ ] can there be multile regions of free memory to allocate page frames for?
- [x] taskes, schedule(), yield() etc
- [ ] time slices for scheduler
- [x] figure out why it when DDEBUG flag is off. why is it trying to run the kernellog command when i say NO WHY
- [x] figure out how to kfree the kmallocs in task.c



# TODO implementations

void* vmm_create_new_context(void)
    purpose: creates a new, blank page map level 4 (pml4) or page directory.
    logic: allocates a page for the top-level structure. crucially, it must map the kernel
    into this new context (usually the higher half)
    so that interrupts work regardless of which task is running.

void vmm_switch_context(void* pml4_phys_addr)
    purpose: context switching.
    implementation: on x86, this writes the physical address of the new directory into the cr3 register.

void vmm_destroy_context(void* pml4_phys_addr)
    purpose: clean up when a task dies.
    logic: frees the page tables associated with this task (but be careful not to free the kernel's pages!).

void vmm_page_fault_handler(registers_t* regs)
    purpose: called by the isr (interrupt service routine) when interrupt 14 (on x86) fires.
    logic:
        read cr2 (on x86) to find the address that caused the fault.
        check error code (was it a permission violation? was the page not present?).
        if valid (e.g., copy-on-write or lazy loading), fix the mapping and return.
        if invalid (e.g., null pointer dereference), terminate the task.
