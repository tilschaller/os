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
          pt_2[j] = (j * 0x200000 + i * 0x40000000) | PAGE_HUGE | PAGE_WRITE | PAGE_PRESENT;
      }
      pt_3[i] = _pt_2 | PAGE_WRITE | PAGE_PRESENT;
  }
}

// declared in kernel/include/kernel/mman.h
// for now flags is ignored, we just set present, read and write flags on any page
void *mmap(void *_addr, void *addr, size_t length, int flags) {
  // supress the unused warning, because it is intentional
  (void)(flags);
  // we store the pointers to the diffrent paging tables in an array 
  // starting from 0 (pt_4) to 3 (pt_1)
  uint64_t *pt[4] = {0};
  // first we find the pointer to the page table (4th level)
  pt[0] = (uint64_t*)(get_cr3_value() + HIGHER_HALF_MIRROR);

  // then we figure out the entries into the different page tables
  /*
    Bits 63 to 48, not used in address translation.
    Bits 47 ... 39 are the PML4 entry.
    Bits 38 ... 30 are the PDPR entry.
    Bits 29 ... 21 are the PD entry.
    Bits 20 ... 12 are the PT entry.
    Offset in the page table.
  */
  // NOTE: these are the start address of the memory segment
  uint16_t pt_entries[4] = {0};
  pt_entries[0] = ((uint64_t)addr >> 39) & 0xFFFF; 
  pt_entries[1] = ((uint64_t)addr >> 30) & 0xFFFF;
  pt_entries[2] = ((uint64_t)addr >> 21) & 0xFFFF;
  pt_entries[3] = ((uint64_t)addr >> 12) & 0xFFFF;

  // is the entry in highest level page empty
  // -> check if Present bit is set (must be bigger then zero)
  if (pt[0][pt_entries[0]] > 0) {
    // read out the pointer to pt_3 (pt[1])
    // mask: 0x000FFFFFFFFFF000
    // we can just assume this ONLY has default bits set, 
    // because we dont deal with flags yet
    pt[1] = (uint64_t*)(pt[0][pt_entries[0]] & 0x000FFFFFFFFFF000);
    // get the virtual address
    pt[1] += HIGHER_HALF_MIRROR;
  } else {
    // alloc a new page with get_page_phys() - this is a physical address
    pt[1] = get_page_phys();
    // write into pt_4 with proper permissions
    pt[0][pt_entries[0]] = (uint64_t)pt[1] | PAGE_DEFAULT;
    // increment pt[1] to virtual address
    // TODO: this can cause an integer overflow i think
    pt[1] = (uint64_t*)((uint64_t)pt[1]  + HIGHER_HALF_MIRROR);
    // zero out pt[1]
    memset(pt[1], 0, PAGE_SIZE);
  }

  // check pt_2 the same as pt_3 even though it could be we just allocated the Page
  // this keeps things simpler
  // you could also only check if this exists, when pt_3 wasnt just allocated
  // for functionality refer to the branch above
  if (pt[1][pt_entries[1]] > 0) {
    pt[2] = (uint64_t*)(pt[1][pt_entries[1]] & 0x000FFFFFFFFFF000);
    pt[2] += HIGHER_HALF_MIRROR;
  } else {
    pt[2] = get_page_phys();
    pt[1][pt_entries[1]] = (uint64_t)pt[2] | PAGE_DEFAULT;
    pt[2] = (uint64_t*)((uint64_t)pt[2]  + HIGHER_HALF_MIRROR);
   memset(pt[2], 0, PAGE_SIZE);
  }

  // TODO: move these 3 similar if branches into a for loop with 3 iterations
  // this is possible, because the segment to be mapped should not cross a pt_3 border (for now)
  // (1 GiB) or less if not aligned
  if (pt[2][pt_entries[2]] > 0) {
    pt[3] = (uint64_t*)(pt[2][pt_entries[2]] & 0x000FFFFFFFFFF000);
    pt[3] += HIGHER_HALF_MIRROR;
  } else {
    pt[3] = get_page_phys();
    pt[2][pt_entries[2]] = (uint64_t)pt[3] | PAGE_DEFAULT;
    pt[3] = (uint64_t*)((uint64_t)pt[3]  + HIGHER_HALF_MIRROR);
   memset(pt[3], 0, PAGE_SIZE);
  }
  
  // just map single pages for now because im lazy and want to work on other things
  if (length != PAGE_SIZE) {
    printf("TODO: we can only map single pages in the address space\n");
    return 0;
  }

  pt[3][pt_entries[3]] = (uint64_t)_addr | PAGE_DEFAULT;

  // confirm everything worked as intended
  return addr;
}
