#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

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

  printf("Reached end of kernel code\n");
  abort();
}
