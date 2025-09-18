#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <kernel/mman.h>

#include "pmm.h"
#include "vmm.h"

size_t p_mmap_entries = 0;
memory_map *p_mmap = 0;

// this function should only be used, before there is a usable pmm setup
uint64_t pre_mmap_get_page(memory_map* mmap, const int entries_c) {
    for (int i = 0; i < entries_c; i++) {
        // memory should be at least one page size big, available
        // and under 2MB, because that is mapped and the kernel isnt in the way
        //printf("mem base: %x, %x, %x\n", mmap[i].base_addr, mmap[i].type, mmap[i].length);
        if (mmap[i].length >= PAGE_SIZE
            && mmap[i].type == BOOT_MEMORY_AVAILABLE
            && mmap[i].base_addr <= 0x200000
            && mmap[i].base_addr != 0
        ) {
                // remove the page from the total size
                mmap[i].length -= PAGE_SIZE;
                // increase the base addr
                mmap[i].base_addr += PAGE_SIZE;
                // return a pointer to the just removed page
                return mmap[i].base_addr - PAGE_SIZE;
        }
    }
    // if no suitable page was found
    printf("TODO: IN: vmm_map_physical_to_high: Not enough mapped physical memory available \n");
    abort();
}

// base must be a valid virtual address
void pm_chunk_create(uint64_t base, uint64_t length) {
  // calculate the number of pages in this memory segment
  // divide by 0x1000, 4096
  int pages = length >> 12;

  // ensure pages is a multiple of eigth
  while (pages++%8 != 0) {}

  // create pm_chunk header at
  pm_chunk *chunk = (pm_chunk*)base;

  // TODO: add check if bitmap would be larger than the first page
  // needed size: ~ PAGE_SIZE * 8 * PAGE_SIZE

  // set the usable addr of this chunk after the first page
  chunk->addr = (uint64_t)base + PAGE_SIZE;
  chunk->length = length;
  chunk->pages = pages;

  // clear the pages in the bitmap
  memset(chunk->bitmap, PM_FREE, pages >> 3);
}

// declared in kernel/include/kernel/mman.h
// return a single physical page of size 0x1000
void *get_page_phys(void) {
    // loop through all the memory segments
    for (size_t i = 0; i < p_mmap_entries; i++) {
        // is the memory usable ?
        if (p_mmap[i].type != BOOT_MEMORY_AVAILABLE) continue;
        // is the segment long enough ?
        if (p_mmap[i].length < PAGE_SIZE) continue;
        // get the bitmap
        uint64_t _segment = p_mmap[i].base_addr;
        pm_chunk *segment = (pm_chunk*)(_segment + HIGHER_HALF_MIRROR);

        // for every (page / 8) uint8_t, check if free
        for (size_t j = 0; j < segment->pages >> 3; j++) {
            // in the uint8_t all pages are used
            if (segment->bitmap[j] == 0xFF) continue;
            // loop through every page in the uint8_t
            // !! the least significant bit is the first page
            for (size_t k = 0; k < 8; k++) {
                if ((segment->bitmap[j] >> k & 1) == PM_USED) continue;
                // mark the page as used
                segment->bitmap[j] |= PM_USED << k;
                // calculate and return the address of the page
                // we just marked as used
                return (void *)(p_mmap[i].base_addr + j * 8 * PAGE_SIZE + k * PAGE_SIZE);
            }
        }
    }
    // if no page was found return NULL
    return NULL;
}
