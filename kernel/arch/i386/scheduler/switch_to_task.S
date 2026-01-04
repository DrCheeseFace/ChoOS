#include <kernel/task.h>
#include <kernel/gdt.h>

.global _switch_to_task
.type _switch_to_task, @function

.extern tss_entry

_switch_to_task:
        pushl %ebx
        pushl %esi
        pushl %edi
        pushl %ebp

        movl current_task_TCB, %edi // set edi to current task TCB
        movl %esp, TCB_OFFSET_ESP(%edi) // esp = current_task.esp

        // load next task's state
        // esi = parameter passed
        movl    20(%esp), %esi

        movl %esi, current_task_TCB // current_task_TCB = esi (passes parameter)


        // set TCB struct members
        movl TCB_OFFSET_ESP(%esi), %esp
        movl TCB_OFFSET_CR3(%esi), %eax
        movl TCB_OFFSET_ESP0(%esi), %ebx

        movb $TASK_STATE_RUNNING, TCB_OFFSET_STATE(%esi)

        // no fucking clue
        movl %ebx, tss_entry + TSS_OFFSET_ESP0

        //eax = previous task VAS
        movl %cr3, %ecx
        cmpl %ecx, %eax
        je .doneVAS
        movl %eax, %cr3

.doneVAS:
        pop %ebp
        pop %edi
        pop %esi
        pop %ebx

        ret
