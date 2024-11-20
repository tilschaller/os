#include <kernel/memory.h>
#include <kernel/multiboot2.h>
#include <stddef.h>
#include <stdint.h>

struct multiboot_tag_mmap *memmap;
struct multiboot_tag_framebuffer *framebuffer_info;

void parse_multiboot(void *multiboot_data) {
  uint32_t *total_size = multiboot_data;
  void *end_ptr = incptr(multiboot_data, *total_size);
  struct multiboot_tag *tag =
      incptr(multiboot_data, sizeof(struct multiboot_tag));

  while (tag) {
    switch (tag->type) {
    case 6:
      memmap = (struct multiboot_tag_mmap *)tag;
      break;
    case 8:
      framebuffer_info = (struct multiboot_tag_framebuffer *)tag;
      break;
    }
    if (incptr(tag, tag->size) >= end_ptr) {
      tag = NULL;
    } else {
      int padding = (8 - (tag->size % 8)) % 8;
      tag = incptr(tag, tag->size +
                            padding); /* to keep the address 8 byte aligned */
    }
  }
}
