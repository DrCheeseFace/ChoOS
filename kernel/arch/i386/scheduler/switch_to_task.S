#include <kernel/task.h>
#include <kernel/gdt.h>

.global _switch_to_task
.type _switch_to_task, @function

.extern tss_entry

_switch_to_task:
        // save old task registers
        pushl %ebx
        pushl %esi
        pushl %edi
        pushl %ebp

        movl current_task_TCB, %edi // set edi = current_task_TCB
        movl %esp, TCB_OFFSET_ESP(%edi) // current_task_TCB = esp

        // load next task's state
        // esi = parameter passed
        movl    20(%esp), %esi

        movl %esi, current_task_TCB // current_task_TCB = esi (passes parameter)


        // set TCB struct members
        movl TCB_OFFSET_ESP(%esi), %esp
        movl TCB_OFFSET_CR3(%esi), %eax
        movl TCB_OFFSET_ESP0(%esi), %ebx

        // set current_task_TCB->state = RUNNING
        movb $TASK_STATE_RUNNING, TCB_OFFSET_STATE(%esi)

        // set task segment selector esp0
        movl %ebx, tss_entry + TSS_OFFSET_ESP0

        // eax = previous task VAS
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

        ret // return to new task
