.global _tss_flush
.type _tss_flush, @function

_tss_flush:
    movl $0x2B, %eax
    LTR %ax
    RET
