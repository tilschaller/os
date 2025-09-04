#ifndef _ACPI_H
#define _ACPI_H 

#include <stdint.h>

typedef struct {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdt_address;
    // only valid for v2
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extenden_checksum;
    uint8_t reserved[3];
} __attribute__((packed)) rsdp_descriptor_t;

rsdp_descriptor_t *find_rsdp(void);

#endif // _ACPI_H
