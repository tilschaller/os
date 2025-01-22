#include <limine.h>

#ifndef _MMAN_H_
#define _MMAN_H_

uint64_t find_mem(struct limine_memmap_entry **entries, int entry_count);
void init_paging(uint64_t free_mem, uint64_t framebuffer_addr, uint64_t phys_kernel_addr);
//int init_vmm();

#endif