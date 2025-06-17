#include <kernel/interrupts.h>

static inline void cli() {
    asm volatile("cli");
}
static inline void sti() {
    asm volatile("sti");
}

void interrupts_initialize() {
}
