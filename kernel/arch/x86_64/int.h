#ifndef _IDT_H
#define _IDT_H 

#include <stdint.h>

typedef struct {
  uint16_t  isr_low;
  uint16_t  kernel_cs;
  uint8_t   ist;
  uint8_t   attributes;
  uint16_t  isr_mid;
  uint32_t  isr_high;
  uint32_t  reserved;
} __attribute__((packed)) idt_entry;

typedef struct {
  uint16_t  limit;
  uint64_t  base;
} __attribute__((packed)) idtr;

typedef struct {
    uint64_t r15;
    uint64_t r14;
    //other pushed registers
    uint64_t rbx;
    uint64_t rax;

    uint64_t vector_number;
    uint64_t error_code;

    uint64_t iret_rip;
    uint64_t iret_cs;
    uint64_t iret_flags;
    uint64_t iret_rsp;
    uint64_t iret_ss;
} cpu_status_t;

#endif 
