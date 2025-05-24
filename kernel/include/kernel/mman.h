#ifndef _MMAN_H_K
#define _MMAN_H_K

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 0x1000

// this functions must be called before any other calls to memory managment functions
void mman_initialize(const uint32_t _mbi);

// returns a pointer to a single physical page
void *get_page_phys(void);

#endif
