#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <limine.h>
#include <ssfn.h>
#include <stdlib.h>
#include <mman.h>
#include <stdio.h>
#include <string.h>
#include <gdt.h>

__attribute__((used, section(".limine_requests"))) 
static volatile LIMINE_BASE_REVISION(3);

__attribute__((used, section(".limine_requests"))) 
static volatile struct limine_framebuffer_request framebuffer_request = {
  .id = LIMINE_FRAMEBUFFER_REQUEST, 
  .revision = 0};

__attribute__((used, section(".limine_requests_start"))) 
static volatile LIMINE_REQUESTS_START_MARKER;

__attribute__((used, section(".limine_requests_end"))) 
static volatile LIMINE_REQUESTS_END_MARKER;

extern unsigned char _binary_u_vga16_sfn_start;

// Kernel entry point
__attribute__((noreturn)) 
void kmain_early(void) {
  asm("cli");

  // Ensure the bootloader actually understands our base revision (see spec).
  if (LIMINE_BASE_REVISION_SUPPORTED == false) {
    abort();
  }

  // Ensure we got a framebuffer.
  if (framebuffer_request.response == NULL || framebuffer_request.response->framebuffer_count < 1) {
    abort();
  }

  struct limine_framebuffer *framebuffer = framebuffer_request.response->framebuffers[0];

  // setting up the printf function
  ssfn_src = (ssfn_font_t *)&_binary_u_vga16_sfn_start;
  ssfn_dst.ptr = framebuffer->address;
  ssfn_dst.w = framebuffer->width;
  ssfn_dst.h = framebuffer->height;
  ssfn_dst.p = framebuffer->pitch;
  ssfn_dst.x = ssfn_dst.y = 0;
  ssfn_dst.fg = 0xFFFFFF;

  printk("---------------------\n");
  printk("---Starting kernel---\n");
  printk("---------------------\n\n");

  init_gdt();

  init_mman(); 
  // control continues from init_mman()
  // that function jumps to kmain - from that point kalloc (mmap) is usable
}

__attribute__((noreturn)) void kmain() {
  exit();
}
