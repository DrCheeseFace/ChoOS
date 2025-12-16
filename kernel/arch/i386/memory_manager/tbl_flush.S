.global _tbl_flush
.type _tbl_flush, @function

_tbl_flush:
        movl 4(%esp), %eax
        invlpg (%eax)
        ret
