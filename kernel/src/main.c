#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <limine.h>

#define SSFN_CONSOLEBITMAP_TRUECOLOR
#include <ssfn.h>

#include <int.h>
#include <kstdio.h>
#include <mman.h>

#include <string.h>

__attribute__((used, section(".limine_requests"))) static volatile LIMINE_BASE_REVISION(3);

__attribute__((used,
               section(".limine_requests"))) static volatile struct limine_framebuffer_request framebuffer_request = {
    .id = LIMINE_FRAMEBUFFER_REQUEST, .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_memmap_request memmap_request = {
    .id = LIMINE_MEMMAP_REQUEST, .revision = 0};

__attribute__((used, section(".limine_requests"))) static volatile struct limine_hhdm_request hhdm_request = {
    .id = LIMINE_HHDM_REQUEST, .revision = 0};

__attribute__((
    used, section(".limine_requests"))) static volatile struct limine_executable_address_request exe_addr_request = {
    .id = LIMINE_EXECUTABLE_ADDRESS_REQUEST, .revision = 0};

__attribute__((used, section(".limine_requests_start"))) static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) static volatile LIMINE_REQUESTS_END_MARKER;

// Halt and catch fire function.
static void hcf(void)
{
    for (;;)
    {
        asm("hlt");
    }
}

extern unsigned char _binary_u_vga16_sfn_start;

// Kernel entry point
__attribute__((noreturn)) void kmain_early(void)
{
    asm("cli");

    // Ensure the bootloader actually understands our base revision (see spec).
    if (LIMINE_BASE_REVISION_SUPPORTED == false)
    {
        hcf();
    }

    // Ensure we got a framebuffer.
    if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1)
    {
        hcf();
    }

    // retrieving data from bootloader
    uint64_t limine_hhdm_offset = hhdm_request.response->offset;
    struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];
    struct limine_memmap_entry **entries = memmap_request.response->entries;
    int entry_count = memmap_request.response->entry_count;
    uint64_t phys_kernel_addr = exe_addr_request.response->physical_base;

    // setting up the printf function
    ssfn_src = (ssfn_font_t *)&_binary_u_vga16_sfn_start;
    ssfn_dst.ptr = framebuffer->address;
    ssfn_dst.w = framebuffer->width;
    ssfn_dst.h = framebuffer->height;
    ssfn_dst.p = framebuffer->pitch;
    ssfn_dst.x = ssfn_dst.y = 0;
    ssfn_dst.fg = 0xFFFFFF;

    kprintf("---------------------\n");
    kprintf("---Starting kernel---\n");
    kprintf("---------------------\n\n");

    kprintf("Initializing Memory Manager\n");

    uint64_t free_mem = find_mem(entries, entry_count);
    if (free_mem == 0)
    {
        kprintf("Failed to find suitable RAM");
        hcf();
    }

    if (init_paging(free_mem, (uint64_t)framebuffer->address - limine_hhdm_offset, phys_kernel_addr) != 0)
    {
        kprintf("Error when enabling paging");
        hcf();
    }
    hcf();
}

void multitasking_test()
{
    kprintf("Multitasking is working!");
    // CLOSE THREAD
}

__attribute__((noreturn)) void kmain()
{
    kprintf("Early setup finished, now inside kmain\n");

    if (init_int() != 0)
    {
        kprintf("Failed to set up IDT");
        hcf();
    }

    asm volatile("sti");
    kprintf("Interrupts enabled\n");

    hcf();
}
