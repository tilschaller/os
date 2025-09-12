#include "acpi.h"
#include <stdbool.h>
#include <stdint.h>
#include <kernel/mman.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static bool validate_rsdp(rsdp_descriptor_t *rsdp) {
    // check for version
    char *byte_array = (char*)rsdp;
    // always validate the first part of the rsdp
    uint32_t sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += byte_array[i];
    }
    // check if last byte is zero else return false
    if ((sum & 0xFF) != 0) return false;
    // the first half is now validated. this is enough for first version 
    if (rsdp->revision == 0) return true;

    // check second half only on version 2
    sum = 0;
    for (int i = 20; i < 36; i++) {
        sum += byte_array[i];
    }
    // same as above
    if ((sum & 0xFF) != 0) return false;
    // all checks passed
    return true; 
 }

rsdp_descriptor_t *find_rsdp(void) {
    // try to find it in first Kb of edba
    uint64_t ebda_seg = *(uint16_t*)(0x40E + HIGHER_HALF_MIRROR);
    uint64_t ebda_addr = (ebda_seg << 4) + HIGHER_HALF_MIRROR;

    for (uint64_t addr = ebda_addr; addr < ebda_addr + 1024; addr += 16) {
        if (!memcmp((void*)addr, "RSD PTR ", 8)) {
            rsdp_descriptor_t *rsdp = (rsdp_descriptor_t*)addr;
            if (validate_rsdp(rsdp)) return rsdp; 
        }
    }

    // try to find in BIOS area
    for (uint64_t addr = 0xE0000 + HIGHER_HALF_MIRROR; addr < 0x100000 + HIGHER_HALF_MIRROR; addr += 16) {
        if (!memcmp((void*)addr, "RSD PTR ", 8)) {
            rsdp_descriptor_t *rsdp = (rsdp_descriptor_t*)addr;
            if (validate_rsdp(rsdp)) return rsdp; 
        }
    }

    // couldnt be found
    fprintf(debug, "ERROR: Could not find RSDP\n");
    printf("ERROR: could not find RSDP\n");
    abort();
    __builtin_unreachable();
}

uint64_t find_dt(rsdp_descriptor_t *rsdp, const char signature[4]) {
    // do we need to use xsdt?
    if (rsdp->revision > 0 && rsdp->xsdt_address != 0) {
        xsdt_t *xsdt = (xsdt_t*)(rsdp->xsdt_address + HIGHER_HALF_MIRROR);
        // calculate the number of entries in xsdt
        size_t number_of_items = (xsdt->sdt_header.length - sizeof(acpi_sdt_header_t)) / 8;
        // check each entry for the signature
        for (size_t i = 0; i < number_of_items; i++) {
            acpi_sdt_header_t *dt = (acpi_sdt_header_t*)(xsdt->sdt_addresses[i] + HIGHER_HALF_MIRROR);
            // are the signatures the same
            if (!memcmp(dt->signature, signature, 4)) return (uint64_t)dt;
        }
    } // else we use rsdt 
    else {
        rsdt_t *rsdt = (rsdt_t*)(rsdp->rsdt_address + HIGHER_HALF_MIRROR);
        // calculate the number of entries in rsdt
        size_t number_of_items = (rsdt->sdt_header.length - sizeof(acpi_sdt_header_t)) / 4;
        // check each entry for the signature
        for (size_t i = 0; i < number_of_items; i++) {
            acpi_sdt_header_t *dt = (acpi_sdt_header_t*)(rsdt->sdt_addresses[i] + HIGHER_HALF_MIRROR);
            // are the signatures the same
            if (!memcmp(dt->signature, signature, 4)) return (uint64_t)dt;
        }
    }
    return 0;
}
