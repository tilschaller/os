#include <kernel/boot.h>
#include <kernel/multiboot.h>
#include <kernel/mman.h>
#include <kernel/util.h>
#include "int.h"

// i did not know where to put this
extern void mman_initialize(const uint32_t _mbi);


void arch_init(const uint32_t _mbi) {
    mman_initialize(_mbi);

    // marks the very first page as used
    // because it could lead to confusion,
    // because this function returns NULL if no page was found
    // and also returns 0 on first call
    get_page_phys();

    interrupts_initialize();
}
