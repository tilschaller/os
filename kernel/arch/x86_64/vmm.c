#include <kernel/mman.h>
#include <kernel/multiboot.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vmm.h"
#include "boot.h"

uint64_t get_cr3_value(void) {
  uint64_t cr3_value;
  __asm__ __volatile__(
    "mov %%cr3, %%rax\n\t"
    "mov %%rax, %0\n\t"
    :"=m" (cr3_value)
    :
    : "%rax"
  );
  return cr3_value;
}

// this function should only be used, before there is a usable pmm setup
static uint64_t mmap_get_page(memory_map* mmap, const int entries_c) {
    for (int i = 0; i < entries_c; i++) {
        // memory should be at least one page size big, available
        // and under 2MB, because that is mapped and the kernel isnt in the way
        //printf("mem base: %x, %x, %x\n", mmap[i].base_addr, mmap[i].type, mmap[i].length);
        if (mmap[i].length >= PAGE_SIZE
            && mmap[i].type == MULTIBOOT_MEMORY_AVAILABLE
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

// pointers to physical memory is marked with an underscore
void vmm_map_physical_to_high(memory_map* mmap, const int entries_c) {
  uint64_t highest = mmap[entries_c - 1].length + mmap[entries_c - 1].length;
  printf("Highest physical address: 0x%x\n", highest);

  // round up highest to the nearest HUGE_PAGE_SIZE value
  highest = ((highest + 0x2000000 - 1) / 0x200000) * 0x200000;

  // get first entry into the page table
  uint64_t *pt_4 = (uint64_t*)(get_cr3_value() + KERNEL_VMA);

  // get a unused page
  uint64_t _pt_3 = mmap_get_page(mmap, entries_c);
  uint64_t *pt_3 = (uint64_t*)(_pt_3 + KERNEL_VMA);
  memset(pt_3, 0, PAGE_SIZE);

  // write pt_3 into pt_4
  pt_4[256] = _pt_3 | PAGE_WRITE | PAGE_PRESENT;

  // amount of huge pages needed to map physical memory
  uint64_t pages_c = ((highest / 0x200000) + 510) >> 9;
  if (pages_c > 512) {
      printf("TODO: IN: vmm.c: highest phys addr is bigger than 512 GB");
  }

  for (size_t i = 0; i < pages_c; i++) {
      uint64_t _pt_2 = mmap_get_page(mmap, entries_c);
      uint64_t *pt_2 = (uint64_t*)(_pt_2 + KERNEL_VMA);
      for (size_t j = 0; j < 512; j++) {
          pt_2[j] = (j*0x200000)+(i*0x40000000) | PAGE_HUGE | PAGE_WRITE | PAGE_PRESENT;
      }
      pt_3[i] = _pt_2 | PAGE_WRITE | PAGE_PRESENT;
  }
}
