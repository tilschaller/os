#include <kernel/mman.h>
#include <kernel/multiboot.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "boot.h"
#include "pmm.h"
#include "vmm.h"

void mman_initialize(const uint32_t _mbi) {
    // mbi should still be in the first 4MB of physical memory
    // this means, we can just add KERNEL_VMA
    if (_mbi >= 1023 * PAGE_SIZE) {
        printf("TODO: multiboot info structure is not in mapped memory, map the adress beforehand\n");
    }
    const multiboot_info *mbi = (multiboot_info*)(_mbi + KERNEL_VMA);
    // ensure the bootloader actually provided a memmap
    // bit 2 of flags has to be 1, if not abort()
    if (mbi->flags >> 2 == 0) {
        printf("Bootloader did not provide memory map\n");
        abort();
    };

    // calculate the total number of mempap entries
    const int entries_c = mbi->mmap_length / sizeof(memory_map);

    memory_map *mmap = (memory_map*)(mbi->mmap_addr + KERNEL_VMA);

    // map all the physical memory to a predetermined address (HIGHER_HALF_MIRROR in vmm.h)
    vmm_map_physical_to_high(mmap, entries_c);

    for (int i = 0; i < entries_c; i++) {
        pm_chunk_create(mmap[i].base_addr + HIGHER_HALF_MIRROR, mmap[i].length);
    }
}
