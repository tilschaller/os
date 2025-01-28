#include <int.h>
#include <stdint.h>
#include <kstdio.h>
#include <stdbool.h>

extern void hcf();

__attribute__((noreturn))
void exception_handler(void);
void keyboard_input_handler(void);

struct IDTEntry{
    uint16_t offset_low;    // Lower 16 bits of the handler function address
    uint16_t selector;      // Code segment selector in the GDT
    uint8_t  ist : 3;       // Interrupt Stack Table (0 if not used)
    uint8_t  zero : 5;      // Reserved (set to 0)
    uint8_t  type_attr;     // Type and attributes (e.g., interrupt gate, DPL, present)
    uint16_t offset_mid;    // Middle 16 bits of the handler function address
    uint32_t offset_high;   // Higher 32 bits of the handler function address (64-bit mode only)
    uint32_t reserved;      // Reserved (set to 0)
} __attribute__((packed));  // Ensure no padding is added

struct IDTR {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

__attribute__((aligned(16))) struct IDTEntry idt[256];

struct IDTR idtr = {
    .limit = sizeof(idt) - 1,
    .base = (uint64_t)&idt,
};

extern void* isr_stub_table[];

void set_idt_entry(int vector, void (*handler)(), uint16_t selector, uint8_t type_attr) {
    uint64_t handler_address = (uint64_t)handler;
    idt[vector].offset_low = handler_address & 0xFFFF;         // Lower 16 bits
    idt[vector].selector = selector;                          // Code segment selector
    idt[vector].ist = 0;                                      // No special IST
    idt[vector].zero = 0;                                     // Reserved
    idt[vector].type_attr = type_attr;                        // Type and attributes
    idt[vector].offset_mid = (handler_address >> 16) & 0xFFFF; // Middle 16 bits
    idt[vector].offset_high = (handler_address >> 32)  & 0xFFFFFFFF;        // Higher 32 bits
    idt[vector].reserved = 0;                                 // Reserved
}

static inline void outb(uint16_t port, uint8_t val) {
    __asm__ volatile ( "outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ volatile ( "inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

#define PIC1		0x20		/* IO base address for master PIC */
#define PIC2		0xA0		/* IO base address for slave PIC */
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

#define PIC_EOI		0x20		/* End-of-interrupt command code */

#define ICW1_ICW4	0x01		/* Indicates that ICW4 will be present */
#define ICW1_SINGLE	0x02		/* Single (cascade) mode */
#define ICW1_INTERVAL4	0x04		/* Call address interval 4 (8) */
#define ICW1_LEVEL	0x08		/* Level triggered (edge) mode */
#define ICW1_INIT	0x10		/* Initialization - required! */

#define ICW4_8086	0x01		/* 8086/88 (MCS-80/85) mode */
#define ICW4_AUTO	0x02		/* Auto (normal) EOI */
#define ICW4_BUF_SLAVE	0x08		/* Buffered mode/slave */
#define ICW4_BUF_MASTER	0x0C		/* Buffered mode/master */
#define ICW4_SFNM	0x10		/* Special fully nested (not) */

/*
arguments:
	offset1 - vector offset for master PIC
		vectors on the master become offset1..offset1+7
	offset2 - same for slave PIC: offset2..offset2+7
*/
void PIC_remap(int offset1, int offset2)
{
	
	outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);  // starts the initialization sequence (in cascade mode)
	io_wait();
	outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
	io_wait();
	outb(PIC1_DATA, offset1);                 // ICW2: Master PIC vector offset
	io_wait();
	outb(PIC2_DATA, offset2);                 // ICW2: Slave PIC vector offset
	io_wait();
	outb(PIC1_DATA, 4);                       // ICW3: tell Master PIC that there is a slave PIC at IRQ2 (0000 0100)
	io_wait();
	outb(PIC2_DATA, 2);                       // ICW3: tell Slave PIC its cascade identity (0000 0010)
	io_wait();
	
	outb(PIC1_DATA, ICW4_8086);               // ICW4: have the PICs use 8086 mode (and not 8080 mode)
	io_wait();
	outb(PIC2_DATA, ICW4_8086);
	io_wait();

  //outb(0x36, 0x43); // 0x36 is the control word, 0x43 is the command port

  // Set the divisor for 100 Hz (divisor = 11931)
  //outb(0x1F, 0x40); // Low byte
  //outb(0x2E, 0x40); // High byte
	
	outb(PIC1_DATA, ~(0x02));
	outb(PIC2_DATA, ~(0x00));
}

int init_int() {
  PIC_remap(0x20, 0x28);

  for (int i = 0; i < 32; i++) {
    set_idt_entry(i, isr_stub_table[i], 0x28, 0x8e);
  }
  set_idt_entry(0x21, isr_stub_table[32], 0x28, 0x8e);

  asm volatile("lidt %0" :: "m"(idtr));		
  
  asm volatile("sti");

  return 0;
}

void exception_handler() {
    kprintf("Fatal error occured - halting");
    __asm__ volatile ("cli; hlt"); // Completely hangs the computer
}

void keyboard_input_handler() {
  uint8_t scancode = inb(0x60);
  
  if (scancode <= 0x80) {
   kprintf("Pressed key\n");  
  }

  outb(PIC1_COMMAND, PIC_EOI);

  return;
}
