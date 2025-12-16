.global _tlb_flush
.type _tlb_flush, @function

_tlb_flush:
        movl 4(%esp), %eax
        invlpg (%eax)
        ret
