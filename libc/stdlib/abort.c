#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__)) void abort(const char *err)
{
#if defined(__is_libk)

	if (err != NULL) {
		printf("kernel: panic: %s\n", err);
	}
	else {
		printf("kernel: panic \n");
	}

	__asm__ volatile("cli");

	while (1) {
		__asm__ volatile("hlt");
	}

#else
	// TODO: Abnormally terminate the process as if by SIGABRT.
	printf("abort()\n");
	while (1) {
	}
#endif

	__builtin_unreachable();
}
