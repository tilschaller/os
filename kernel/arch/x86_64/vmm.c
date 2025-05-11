#include <kernel/mman.h>
#include <kernel/multiboot.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pmm.h"
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

// pointers to physical memory is marked with an underscore
void vmm_map_physical_to_high(memory_map* mmap, const int entries_c) {
  uint64_t highest = mmap[entries_c - 1].length + mmap[entries_c - 1].length;
  printf("Highest physical address: 0x%x\n", highest);

  // round up highest to the nearest HUGE_PAGE_SIZE value
  highest = ((highest + 0x2000000 - 1) / 0x200000) * 0x200000;

  // get first entry into the page table
  uint64_t *pt_4 = (uint64_t*)(get_cr3_value() + KERNEL_VMA);

  // get a unused page
  uint64_t _pt_3 = pre_mmap_get_page(mmap, entries_c);
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
      uint64_t _pt_2 = pre_mmap_get_page(mmap, entries_c);
      uint64_t *pt_2 = (uint64_t*)(_pt_2 + KERNEL_VMA);
      for (size_t j = 0; j < 512; j++) {
          pt_2[j] = (j*0x200000)+(i*0x40000000) | PAGE_HUGE | PAGE_WRITE | PAGE_PRESENT;
      }
      pt_3[i] = _pt_2 | PAGE_WRITE | PAGE_PRESENT;
  }
}
