#include <kernel/boot.h>
#include <kernel/multiboot.h>
#include <kernel/mman.h>
#include <kernel/interrupts.h>
#include <kernel/util.h>

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "acpi.h"
#include "pic.h"

void arch_init(const uint32_t _mbi) {
    mman_initialize(_mbi);

    // marks the very first page as used
    // because it could lead to confusion,
    // because this function returns NULL if no page was found
    // and also returns 0 on first call
    get_page_phys();

    interrupts_initialize();

    // acpi unused for now
    rsdp_descriptor_t *rsdp = find_rsdp();

#define PIC_COMMAND_MASTER 0x20
#define PIC_DATA_MASTER 0x21
#define PIC_COMMAND_SLAVE 0xA0
#define PIC_DATA_SLAVE 0xA1

#define ICW_1 0x11
#define ICW_2_M 0x20
#define ICW_2_S 0x28
#define ICW_3_M 0x2
#define ICW_3_S 0x4
#define ICW_4 0x1

    // disable legacy pic
    outb(PIC_COMMAND_MASTER, ICW_1);
    outb(PIC_COMMAND_SLAVE, ICW_1);
    outb(PIC_DATA_MASTER, ICW_2_M);
    outb(PIC_DATA_SLAVE, ICW_2_S);
    outb(PIC_DATA_MASTER, ICW_3_M);
    outb(PIC_DATA_SLAVE, ICW_3_S);
    outb(PIC_DATA_MASTER, ICW_4);
    outb(PIC_DATA_SLAVE, ICW_4);
    outb(PIC_DATA_MASTER, 0xFF);
    outb(PIC_DATA_SLAVE, 0xFF);

    // next we try to find the madt table
    // used to configure apic
    madt_t* madt = (madt_t*)find_dt(rsdp, "APIC");
    if (madt == NULL) {
        fprintf(tty, "Could not find MADT Table used to configure local apic\n");
        // TODO: check if apic is supported (with cpudid 0x1)
        // edx bit 9
        fprintf(tty, "This is currently needed\n");
        abort();
    };
    
    lapic_t *lapic = (lapic_t*)(madt->lapic_addr + HIGHER_HALF_MIRROR);
    // set the spurious interrupt vector bit 8
    lapic->s_i_v = lapic->s_i_v | 0x100;

    // try to find the io_apic. it should always be found
    uint64_t io_apic_addr = 0;
    for (uint64_t i = (uint64_t)madt + sizeof(madt_t); i < (uint64_t)madt + madt->header.length;) {
        madt_record_t *entry = (madt_record_t*)i;
        if (entry->type == 1) {
            io_apic_addr = (uint64_t)entry->record.io_apic.addr;
            break;
        }
        i += entry->length;
    }
}
