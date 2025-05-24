#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <kernel/tty.h>
#include <kernel/multiboot.h>
#include <kernel/mman.h>

void kernel_main(const uint32_t magic, const uint32_t _mbi) {
  // initialize global variable etc. needed to print text
  // never call printf before this, as it could lead tp undefined behaviour
  terminal_initialize();

  if (magic != MULTIBOOT_BOOTLOADER_MAGIC) {
    printf("Kernel not loaded by multiboot 1 compliant bootloader\n");
    abort();
  }

  printf("***********\n");
  printf("*NIGHTC OS*\n");
  printf("***********\n\n");

  mman_initialize(_mbi);

  // test if this works - also marks the very first page as used
  printf("We just fetched a page: %x\n", get_page_phys());

  printf("\nReached end of kernel code\n");
  abort();
}
