#include <stdio.h>
#include <stdlib.h>

static void hcf(void) {
	for (;;) {
		asm ("hlt");
	}
}

__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
	// TODO: Add proper kernel panic.
	printf("\nkernel: panic: abort()\n");
	hcf();
#else
	// TODO: Abnormally terminate the process as if by SIGABRT.
	printf("abort()\n");
#endif
	while (1) { }
	__builtin_unreachable();
}
