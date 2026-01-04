.global _forge_kernel_stack
.type _forge_kernel_stack, @function

# Function: _forge_kernel_stack
# Prototype: uint32_t _forge_kernel_stack(uint32_t stack_top, void (*task)(void), void (*wrapper)(void));
#
# Arguments on stack (Standard C layout):
#   [ESP + 4]  : stack_top  (High address of the buffer)
#   [ESP + 8]  : task_func  (The actual tas code)
#   [ESP + 12] : wrapper    (task_start_up)
#
# Return:
#   EAX bottom of forged stack
_forge_kernel_stack:
        movl 4(%esp), %eax    # eax = stack_top
        movl 8(%esp), %ecx    # ecx = task_func
        movl 12(%esp), %edx   # edx = task_start_up

        subl $4, %eax         # decrement stack pointer
        movl %ecx, (%eax)     # write task_func at new top

        subl $4, %eax         # move the new stack pointer down by 4 bytes.
        movl %edx, (%eax)     # write task_func

        subl $4, %eax         # move the new stack pointer down by 4 bytes.
        movl %ecx, (%eax)     # write task_start_up

        subl $16, %eax        # Move the new stac pointer down 16 bytes.

        xorl %edx, %edx
        movl %edx, (%eax)
        movl %edx, 4(%eax)
        movl %edx, 8(%eax)
        movl %edx, 12(%eax)

        ret
