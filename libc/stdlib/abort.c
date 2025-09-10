#include <stdio.h>
#include <stdlib.h>

__attribute__((__noreturn__))
void abort(void) {
#if defined(__is_libk)
	printf("kernel: panic: abort()\n");
	// completely halt the computer
	__asm__ volatile ("cli; hlt");
#else
	// TODO: Abnormally terminate the process as if by SIGABRT.
#endif
	while (1) { }
	__builtin_unreachable();
}


__attribute__((__noreturn__))
void exit(int exit_status) {
	// TODO: this function should also call all functions registered with an atexit() fucntion which also doesnt exists
#if defined(__is_libk)
	(void)exit_status;
	printf("Exiting gracefully\n");
	// completely halt the computer
	__asm__ volatile ("cli; hlt");
#else
	// TODO: Abnormally terminate the process as if by SIGABRT.
#endif
	while (1) { }
	__builtin_unreachable();
}