.global _forge_kernel_stack
.type _forge_kernel_stack, @function

# Function: _forge_kernel_stack
# Prototype: uint32_t _forge_kernel_stack(uint32_t stack_top, void (*task)(void), void (*wrapper)(void));
#
# Arguments on stack (Standard C layout):
#   [ESP + 4]  : stack_top
#   [ESP + 8]  : task_func
#   [ESP + 12] : wrapper    (task_start_up)
#
# Return:
#   EAX bottom of forged stack
.equ ARG_STACK_TOP_OFFSET, 4
.equ ARG_TASK_FUNC_OFFSET, 8
.equ ARG_WRAPPER_OFFSET, 12
_forge_kernel_stack:
        movl ARG_STACK_TOP_OFFSET(%esp), %eax    # eax = stack_top
        movl ARG_TASK_FUNC_OFFSET(%esp), %ecx    # ecx = task_func
        movl ARG_WRAPPER_OFFSET(%esp), %edx      # edx = task_start_up

        subl $4, %eax         # push task_func to fake stack
        movl %ecx, (%eax)

        subl $4, %eax         # push task_start_up to fake stack
        movl %edx, (%eax)

        subl $16, %eax        # make 4 empty slots

        xorl %edx, %edx
        movl %edx, (%eax)
        movl %edx, ARG_STACK_TOP_OFFSET(%eax)
        movl %edx, ARG_TASK_FUNC_OFFSET(%eax)
        movl %edx, ARG_WRAPPER_OFFSET(%eax)

        ret
