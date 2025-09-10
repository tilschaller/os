#ifndef _ACPI_H
#define _ACPI_H 

#include <stdint.h>

// TODO: mark physical addresses with an underscore (_)
// example: _xsdt_addresses in rsdp_descriptor_t

typedef struct {
    char signature[8];
    uint8_t checksum;
    char OEMID[6];
    uint8_t revision;
    uint32_t rsdt_address;
    // only valid for v2
    uint32_t length;
    uint64_t xsdt_address;
    uint8_t extended_checksum;
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
    uint64_t sdt_addresses[];
} __attribute__((packed)) xsdt_t;

typedef struct {
    acpi_sdt_header_t header; // signature "APIC"
    uint32_t local_apic_addr;
    uint32_t flags;
    // somewhere there are the types
} __attribute__((packed)) madt_t;

rsdp_descriptor_t *find_rsdp(void);
uint64_t find_dt(rsdp_descriptor_t *rsdp, const char signature[4]);

#endif // _ACPI_H
