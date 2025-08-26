#include <stdio.h>

int fputs(output_stream output, const char* string) {
	return fprintf(output, "%s\n", string);
}
