#include <kernel/process.h>
#include <kernel/gdt.h>

.global _switch_to_process
.type _switch_to_process, @function

.extern tss_entry

_switch_to_process:
        cmpl $0, postpone_task_switches_counter
        je .continue
        movl $1, task_switches_postponed_flag
        ret
.continue:
        // save old process registers
        pushl %ebx
        pushl %esi
        pushl %edi
        pushl %ebp

        movl current_process_PCB, %edi // set edi = current_process_PCB
        movl %esp, PCB_OFFSET_ESP(%edi) // current_process_PCB = esp

        // load next process's state
        // esi = parameter passed
        movl    20(%esp), %esi

        movl %esi, current_process_PCB // current_process_PCB = esi (passes parameter)


        // set PCB struct members
        movl PCB_OFFSET_ESP(%esi), %esp
        movl PCB_OFFSET_CR3(%esi), %eax
        movl PCB_OFFSET_ESP0(%esi), %ebx

        // set current_process_PCB->state = RUNNING
        movb $PROCESS_STATE_RUNNING, PCB_OFFSET_STATE(%esi)

        // set process segment selector esp0
        movl %ebx, tss_entry + TSS_OFFSET_ESP0

        // eax = previous process VAS
        movl %cr3, %ecx
        cmpl %ecx, %eax
        je .doneVAS
        movl %eax, %cr3

.doneVAS:
        // remove junk added at the top
        pop %ebp
        pop %edi
        pop %esi
        pop %ebx

        ret // return to new process
