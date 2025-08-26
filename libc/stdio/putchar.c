#include <stdio.h>

#if defined(__is_libk)
#include <kernel/tty.h>
#include <kernel/debug.h>
#endif

int fputchar(output_stream output, int ic) {
#if defined(__is_libk)
	char c = (char) ic;
	switch (output) {
		case tty:
			terminal_write(&c, sizeof(c));
			break;
		case debug:
			debug_write(&c, sizeof(c));
	}
#else
	// TODO: Implement stdio and the write system call.
#endif
	return ic;
}
