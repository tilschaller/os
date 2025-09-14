#ifndef _MMAN_H
#define _MMAN_H

#include <stdint.h>
#include <stddef.h>

#define PAGE_SIZE 0x1000

// returns a pointer to a single physical page (size: 0x1000)
void *get_page_phys(void);

// map a physical address range into the address space
// flags is unused for now
// TODO: make flags usable
void *mmap(void *_addr, void *addr, size_t length, int flags);

#endif
