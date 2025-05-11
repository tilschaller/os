#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>
#include <stddef.h>
#include <kernel/multiboot.h>

#define PMM_FREE 0
#define PMM_USED 1

typedef struct {
  uint64_t addr;
  uint64_t length;
  uint8_t bitmap[];
} pm_chunk;

extern size_t p_mmap_entries;
extern memory_map *p_mmap;

// these functions are only used for initialization of the memory manager
uint64_t pre_mmap_get_page(memory_map* mmap, const int entries_c);
void pm_chunk_create(const uint64_t base, const uint64_t length);

#endif // _PMM_H
