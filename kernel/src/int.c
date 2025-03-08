#include <int.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

extern void hcf();

__attribute__((noreturn)) void exception_handler(void);
void keyboard_input_handler(void);
void system_timer_handler(void);

struct IDTEntry
{
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist : 3;
    uint8_t zero : 5;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t reserved;
} __attribute__((packed));

struct IDTR
{
    uint16_t limit;
    uint64_t base;
} __attribute__((packed));

__attribute__((aligned(16))) struct IDTEntry idt[256];

struct IDTR idtr = {
    .limit = sizeof(idt) - 1,
    .base = (uint64_t)&idt,
};

extern void *isr_stub_table[];

void set_idt_entry(int vector, void (*handler)(), uint16_t selector, uint8_t type_attr)
{
    uint64_t handler_address = (uint64_t)handler;
    idt[vector].offset_low = handler_address & 0xFFFF;
    idt[vector].selector = selector;
    idt[vector].ist = 0;
    idt[vector].zero = 0;
    idt[vector].type_attr = type_attr;
    idt[vector].offset_mid = (handler_address >> 16) & 0xFFFF;
    idt[vector].offset_high = (handler_address >> 32) & 0xFFFFFFFF;
    idt[vector].reserved = 0;
}

static inline void outb(uint16_t port, uint8_t val)
{
    __asm__ volatile("outb %b0, %w1" : : "a"(val), "Nd"(port) : "memory");
}

static inline uint8_t inb(uint16_t port)
{
    uint8_t ret;
    __asm__ volatile("inb %w1, %b0" : "=a"(ret) : "Nd"(port) : "memory");
    return ret;
}

static inline void io_wait(void)
{
    outb(0x80, 0);
}

#define PIC1 0x20 /* IO base address for master PIC */
#define PIC2 0xA0 /* IO base address for slave PIC */
#define PIC1_COMMAND PIC1
#define PIC1_DATA (PIC1 + 1)
#define PIC2_COMMAND PIC2
#define PIC2_DATA (PIC2 + 1)

#define PIC_EOI 0x20

#define ICW1_ICW4 0x01
#define ICW1_SINGLE 0x02
#define ICW1_INTERVAL4 0x04
#define ICW1_LEVEL 0x08
#define ICW1_INIT 0x10

#define ICW4_8086 0x01
#define ICW4_AUTO 0x02
#define ICW4_BUF_SLAVE 0x08
#define ICW4_BUF_MASTER 0x0C
#define ICW4_SFNM 0x10

void PIC_remap(int offset1, int offset2)
{

    outb(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_wait();
    outb(PIC1_DATA, offset1);
    io_wait();
    outb(PIC2_DATA, offset2);
    io_wait();
    outb(PIC1_DATA, 4);
    io_wait();
    outb(PIC2_DATA, 2);
    io_wait();

    outb(PIC1_DATA, ICW4_8086);
    io_wait();
    outb(PIC2_DATA, ICW4_8086);
    io_wait();

    outb(0x36, 0x43);

    outb(0x1F, 0x40);
    outb(0x2E, 0x40);

    outb(PIC1_DATA, ~(0x03));
    outb(PIC2_DATA, ~(0x00));
}

int init_int()
{
    PIC_remap(0x20, 0x28);

    for (int i = 0; i < 32; i++)
    {
        set_idt_entry(i, isr_stub_table[i], 0x28, 0x8e);
    }
    set_idt_entry(0x20, isr_stub_table[32], 0x28, 0x8e);
    set_idt_entry(0x21, isr_stub_table[33], 0x28, 0x8e);

    asm volatile("lidt %0" ::"m"(idtr));

    return 0;
}

void exception_handler()
{
    printk("Fatal error occured - halting\n");
    asm volatile("cli; hlt"); // Completely hangs the computer
}

void keyboard_input_handler()
{
    uint8_t scancode = inb(0x60);

    outb(PIC1_COMMAND, PIC_EOI);

    return;
}

uint64_t time_since_boot = 0; // time since boot in milliseconds

void system_timer_handler()
{
    time_since_boot += 10;

    outb(PIC1_COMMAND, PIC_EOI);
}
