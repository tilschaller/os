#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <kernel/interrupts.h>
#include <kernel/mman.h>
#include "int.h"

static inline void cli(void) {
	__asm__ volatile("cli");
}
static inline void sti(void) {
	__asm__ volatile("sti");
}

// interrupt handler for errors (see int_stub.S)
cpu_status_t *interrupt_dispatch_err(cpu_status_t *context);
cpu_status_t *interrupt_dispatch_err(cpu_status_t *context) {
	// we cant deal with any of this
	switch (context->vector_number) {
		case 13:
			fprintf(debug, "DEBUG: INT_ERR: general protection fault\n");
			abort();
			break;
		case 14:
			fprintf(debug, "DEBUG: INT_ERR: page protection fault\n");
			abort();
			break;
		default:
			fprintf(debug, "DEBUG: INT_ERR: unknown interrupt occurred (%x)\n", context->vector_number);
			abort();
			break;
	}
	return context;
}

// interrupt handler
cpu_status_t *interrupt_dispatch(cpu_status_t *context);
cpu_status_t *interrupt_dispatch(cpu_status_t *context) {
	switch (context->vector_number) {
		default:
			fprintf(debug, "DEBUG: INT: unknown interrupt occurred\n");
			break;
	}
	return context;
}

static void set_descriptor(idt_entry *idt, uint8_t entry, void *isr, uint8_t flags) {
	idt_entry *descriptor = idt + entry;
	
	descriptor->isr_low 	= (uint64_t)isr & 0xFFFF;
	descriptor->kernel_cs 	= 0x08; // this probably shouldnt be hardcoded
	descriptor->ist 	= 0;
	descriptor->attributes  = flags;
	descriptor->isr_mid 	= ((uint64_t)isr >> 16) & 0xFFFF;
	descriptor->isr_high 	= ((uint64_t)isr >> 32) & 0xFFFFFFFF;
	descriptor->reserved 	= 0;
}

// defined in int_stub.S
extern void *isr_stub_table[];

void interrupts_initialize(void) {
	// disabnle interrupts just in case 
	cli();

	// get a page for the idt (0x1000 byte for 256 entries)
	uint64_t _idt = (uint64_t)get_page_phys();
	// elevate to higher half mirror of physical memory and cast to idt_entry type
	idt_entry *idt = (idt_entry*)(_idt + HIGHER_HALF_MIRROR);
	memset(idt, 0, PAGE_SIZE);

	static idtr idtreg;

	idtreg.base = (uint64_t)idt;
	idtreg.limit = (uint16_t)sizeof(idt_entry) * 255; // because 256 entries fit on one page

	for (uint8_t vector = 0; vector < 32; vector++) {
		set_descriptor(idt, vector, isr_stub_table[vector], 0x8E);
	}

	__asm__ volatile ("lidt %0" : : "m"(idtreg));
	sti();
}
