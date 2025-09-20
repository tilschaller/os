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

// returns a pointer to a single physical page (size: 0x1000)
void *get_page_phys(void);

// map a physical address range into the address space
// flags is unused for now
// TODO: make flags usable
void *mmap(void *_addr, void *addr, size_t length, int flags);

#endif
