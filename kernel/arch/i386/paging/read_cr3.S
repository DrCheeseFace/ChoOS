.global _read_cr3

.type _read_cr3, @function

_read_cr3:
        movl %cr3, %eax
        ret
