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
