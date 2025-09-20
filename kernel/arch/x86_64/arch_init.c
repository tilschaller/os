#include <kernel/boot.h>
#include <kernel/mman.h>
#include <kernel/util.h>
#include "int.h"

// i did not know where to put this
extern void mman_initialize();


void arch_init() {
    mman_initialize();

    // marks the very first page as used
    // because it could lead to confusion,
    // because this function returns NULL if no page was found
    // and also returns 0 on first call
    get_page_phys();

    interrupts_initialize();
}
