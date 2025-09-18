#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include <kernel/tty.h>
#include <kernel/mman.h>
#include <kernel/interrupts.h>
#include <kernel/boot.h>

#include <string.h>

void kernel_main(const uint32_t magic) {
  fprintf(debug, "DEBUG: entered kernel_main\n");
  // initialize global variables etc. needed to print text
  // never call printf before this, as it could lead to undefined behaviour
  terminal_initialize();

  if (magic != 0x12345678) {
    fprintf(debug, "DEBUG: Kernel not loaded by bootloader provided with kernel\n");
    abort();
  }

  printf("***********\n");
  printf("*NIGHTC OS*\n");
  printf("***********\n\n");

  arch_init();

  printf("> \n");

  fprintf(debug, "DEBUG: leaving kernel main\n");
  exit(0);
}
