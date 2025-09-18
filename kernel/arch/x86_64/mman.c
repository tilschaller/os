#include <kernel/mman.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boot.h"
#include "pmm.h"
#include "vmm.h"

void mman_initialize() {
    const uint32_t entries_c = *(uint32_t*)(0x8000);

    memory_map *mmap = (memory_map*)(0x8004 + KERNEL_VMA);

    // map all the physical memory to a predetermined address (HIGHER_HALF_MIRROR in vmm.h)
    vmm_map_physical_to_high(mmap, entries_c);

    // if memory segment is usable create a pm_chunk struct at base,
    // + bitmap
    // uses the first page of the memory segment
    for (uint32_t i = 0; i < entries_c; i++) {
        if (mmap[i].type == BOOT_MEMORY_AVAILABLE && mmap[i].length > PAGE_SIZE + 1) {
            pm_chunk_create(mmap[i].base_addr + HIGHER_HALF_MIRROR, mmap[i].length);
        }
    }

    // write this into variables, so pmm.c can access
    p_mmap = mmap;
    p_mmap_entries = entries_c;

    // from here on out pmm.c functions should be usable
    // from here on out vmm.c functions should be usable
}
