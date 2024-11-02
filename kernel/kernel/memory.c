#include <kernel/memory.h>
#include <kernel/multiboot2.h>
#include <stdint.h>

uint64_t kernel_p4[512] __attribute__((aligned(0x1000)));
uint64_t kernel_p3[512] __attribute__((aligned(0x1000)));
uint64_t kernel_p2[512] __attribute__((aligned(0x1000)));
uint64_t kernel_p1[512] __attribute__((aligned(0x1000)));
uint64_t framebuffer_p1[512] __attribute__((aligned(0x1000)));

extern struct multiboot_tag_framebuffer *framebuffer_info;

void init_mman() {
  /*
TODO: Make proper mman: This one only adds framebuffer to the boot paging table
  */
  for (int i = 0; i < ENTRIES_PER_PT; i++) {
    kernel_p4[i] = 0;
    kernel_p3[i] = 0;
    kernel_p2[i] = 0;
  }

  kernel_p4[511] = V2P(&kernel_p3) + (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL);
  kernel_p3[0] = V2P(&kernel_p2) + (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL);
  kernel_p2[0] = V2P(&kernel_p1) + (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL);
  kernel_p2[1] =
      V2P(&framebuffer_p1) + (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL);

  for (int i = 0; i < ENTRIES_PER_PT; i++) {
    kernel_p1[i] = (i << 12) + (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL);
  }
  for (int i = 0; i < ENTRIES_PER_PT; i++) {
    framebuffer_p1[i] =
        (framebuffer_info->common.framebuffer_addr + (i << 12)) +
        (PAGE_PRESENT | PAGE_WRITE | PAGE_GLOBAL);
  }

  asm volatile("movq %0, %%cr3" ::"r"(V2P(&kernel_p4))); // load PDPT into CR3
}
