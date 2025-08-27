#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/mman.h>
#include <kernel/interrupts.h>

void kernel_main(const uint32_t magic, const uint32_t _mbi) {
  fprintf(debug, "DEBUG: entered kernel_main\n");
  // initialize global variables etc. needed to print text
  // never call printf before this, as it could lead to undefined behaviour
  terminal_initialize();

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    printf("Kernel not loaded by multiboot 1 compliant bootloader\n");
    abort();
  }

  printf("***********\n");
  printf("*NIGHTC OS*\n");
  printf("***********\n\n");

  mman_initialize(_mbi);

  // marks the very first page as used
  // because it could lead to confusion,
  // because this function returns NULL if no page was found
  // and also returns 0 on first call
  get_page_phys();

  interrupts_initialize();

  printf("Reached end of kernel code\n");
  fprintf(debug, "DEBUG: leaving kernel main\n");
  abort();
}
