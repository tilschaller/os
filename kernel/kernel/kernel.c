#include <stdint.h>

#include <kernel/memory.h>
#include <kernel/multiboot2.h>
#include <kernel/tty.h>

void kernel_main(uint64_t multiboot_magic, void *multiboot_data) {
  if (multiboot_magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
    parse_multiboot(P2V(multiboot_data));

    init_mman();

    for (;;) {
    }
  }
  return;
}
