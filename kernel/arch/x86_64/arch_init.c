#include <kernel/boot.h>
#include <kernel/multiboot.h>
#include <kernel/mman.h>
#include <kernel/interrupts.h>
#include <kernel/util.h>

#include <stdint.h>
#include <stdio.h>

#include "acpi.h"

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

void arch_init(const uint32_t _mbi) {
    mman_initialize(_mbi);

    // marks the very first page as used
    // because it could lead to confusion,
    // because this function returns NULL if no page was found
    // and also returns 0 on first call
    get_page_phys();

    interrupts_initialize();

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

    rsdp_descriptor_t *rsdp = find_rsdp();
}
