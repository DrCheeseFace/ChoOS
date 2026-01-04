.global _load_page_directory

_load_page_directory:
	pushl	%ebp
	movl	%esp, %ebp

	movl	8(%ebp), %eax
	movl	%eax, %cr3

	movl	%ebp, %esp
	popl	%ebp
	ret
