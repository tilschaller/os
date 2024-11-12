#include <stdint.h>

#include <kernel/memory.h>
#include <kernel/multiboot2.h>
#include <kernel/tty.h>

void kernel_main(uint64_t multiboot_magic, void *multiboot_data) {
  if (multiboot_magic == MULTIBOOT2_BOOTLOADER_MAGIC) {
    parse_multiboot(P2V(multiboot_data));
    init_mman(); // doesnt actually

    void *framebuffer_addr =
        (void *)(0x200000 + KERNEL_OFFSET); // mapped after 2 mb of kernel_data

    for (;;) {
    }
  }
  return;
}
