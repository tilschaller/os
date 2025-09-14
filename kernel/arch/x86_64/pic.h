#ifndef _PIC_H
#define _PIC_H

#include <stdint.h>

typedef struct {
    char reserved[0x20] __attribute__((aligned(0x10)));
    uint32_t lapic_id __attribute__((aligned(0x10)));
    uint32_t lapic_version __attribute__((aligned(0x10)));
    char reserved_2[0x40] __attribute__((aligned(0x10)));
    uint32_t task_prio __attribute__((aligned(0x10)));
    uint32_t arbitrary_prio __attribute__((aligned(0x10)));
    uint32_t processor_prio __attribute__((aligned(0x10)));
    uint32_t eoi __attribute__((aligned(0x10)));
    uint32_t remote_read __attribute__((aligned(0x10)));
    uint32_t logical_dest __attribute__((aligned(0x10)));
    uint32_t dest_format __attribute__((aligned(0x10)));
    uint32_t s_i_v __attribute__((aligned(0x10)));
    // add other registers

} lapic_t;

typedef struct {
    
} io_apic_t;

#endif // _PIC_H
