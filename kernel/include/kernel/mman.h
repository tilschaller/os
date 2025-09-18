#ifndef _MMAN_H
#define _MMAN_H

#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint64_t base_addr;
    uint64_t length;
    uint32_t type;
#define BOOT_MEMORY_AVAILABLE 1
    uint32_t acpi_extended_attributes;
} __attribute__((packed)) memory_map;

#define PAGE_SIZE 0x1000
// TODO: move to arch specific dir
// is this address even accessible on 32 bit for example?
#define HIGHER_HALF_MIRROR 0xffff800000000000

// TODO: move initialization functions to arch specific dir
// this functions must be called before any other calls to memory managment functions
// examples: get_page_phys, mmap
void mman_initialize();

// returns a pointer to a single physical page (size: 0x1000)
void *get_page_phys(void);

// map a physical address range into the address space
// flags is unused for now
// TODO: make flags usable
void *mmap(void *_addr, void *addr, size_t length, int flags);

#endif
