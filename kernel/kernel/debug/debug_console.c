#include <kernel/util.h>
#include <stddef.h>

void debug_putchar(unsigned char c) {
    outb(0xE9, c);
}

void debug_write(const char* data, size_t size) {
    for (size_t i = 0; i < size; i++)
		debug_putchar(data[i]);
}
