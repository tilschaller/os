#include <stdint.h>
#include <kernel/util.h>

void outb(uint16_t port, uint8_t val) {
    // TODO: check if outb is platform specific
    __asm__ volatile("outb %0, %1" : : "a"(val), "Nd"(port));
}
