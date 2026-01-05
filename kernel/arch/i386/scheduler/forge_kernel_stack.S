.global _forge_kernel_stack
.type _forge_kernel_stack, @function

# Function: _forge_kernel_stack
# Prototype: uint32_t _forge_kernel_stack(uint32_t stack_top,
#                                         void (*process)(void),
#                                         void (*process_start_up)(void),
#                                         void(*cleanup_func)(void))
#
# Arguments on stack (Standard C layout):
#   [ESP + 4]  : stack_top
#   [ESP + 8]  : process_func
#   [ESP + 12] : process_start_up
#   [ESP + 16] : cleanup_func
#
# Return:
#   EAX bottom of forged stack
.equ ARG_STACK_TOP_OFFSET, 4
.equ ARG_PROCESS_FUNC_OFFSET, 8
.equ ARG_WRAPPER_OFFSET, 12
.equ ARG_CLEANUP_FUNC_OFFSET, 16
_forge_kernel_stack:
        movl ARG_STACK_TOP_OFFSET(%esp), %eax    # eax = stack_top

        movl ARG_CLEANUP_FUNC_OFFSET(%esp), %edx # edx = cleanup_func
        subl $4, %eax                            # push cleanup_func to fake stack
        movl %edx, (%eax)

        movl ARG_PROCESS_FUNC_OFFSET(%esp), %edx    # edx = process_func
        subl $4, %eax                            # push process_func to fake stack
        movl %edx, (%eax)

        movl ARG_WRAPPER_OFFSET(%esp), %edx      # edx = process_start_up
        subl $4, %eax                            # push process_start_up to fake stack
        movl %edx, (%eax)

        subl $16, %eax        # make 4 empty slots

        # zero out
        xorl %edx, %edx
        movl %edx, (%eax)
        movl %edx, ARG_STACK_TOP_OFFSET(%eax)
        movl %edx, ARG_PROCESS_FUNC_OFFSET(%eax)
        movl %edx, ARG_WRAPPER_OFFSET(%eax)


        ret
