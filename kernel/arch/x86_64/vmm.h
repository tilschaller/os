#ifndef _VMM_H
#define _VMM_H

#include <stdint.h>


#define HIGHER_HALF_MIRROR 0xffff800000000000

#define PAGE_PRESENT 1
#define PAGE_WRITE 1 << 1
#define PAGE_HUGE 1 << 7

#define PAGE_DEFAULT PAGE_WRITE | PAGE_PRESENT

uint64_t get_cr3_value(void);

void vmm_map_physical_to_high(memory_map* mmap, const uint32_t entries_c);

#endif // _VMM_H
