.global _switch_to_task
.type _switch_to_task, @function

.extern tss_entry

.equ PCB_ESP, 0
.equ PCB_ESP0, 4
.equ PCB_CR3, 8
.equ TSS_ESP0, 4


_switch_to_task:
        pushl %ebx
        pushl %esi
        pushl %edi
        pushl %ebp

        // set edi to current process PCB
        movl current_process_PCB, %edi
        // esp = current_process.esp
        movl %esp, PCB_ESP(%edi)

        // load next task's state
        // esi = parameter passed
        movl    20(%esp), %esi

        // current_process_PCB = esi (passes parameter)
        movl %esi, current_process_PCB

        // set PCB struct members
        movl PCB_ESP(%esi), %esp
        movl PCB_CR3(%esi), %eax
        movl PCB_ESP0(%esi), %ebx

        // no fucking clue
        movl %ebx, tss_entry + TSS_ESP0

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
