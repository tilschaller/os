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

typedef struct {
    char signature[4];
    uint32_t length;
    uint8_t revision;
    char oem_id[6];
    char oem_table_id[8];
    uint32_t oem_revision;
    uint32_t creator_id;
    uint32_t creator_revision;
} __attribute__((packed)) acpi_sdt_header_t;

typedef struct {
    acpi_sdt_header_t sdt_header; // signature "RSDT"
    uint32_t sdt_addresses[];
} __attribute__((packed)) rsdt_t;

typedef struct {
    acpi_sdt_header_t sdt_header; // signature "XSDT"
    uint32_t xdt_addresses[];
} __attribute__((packed)) xsdt_t;

rsdp_descriptor_t *find_rsdp(void);

#endif // _ACPI_H
