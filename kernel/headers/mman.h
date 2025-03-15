#ifndef __MMAN_H__
#define __MMAN_H__

#include <stddef.h>
#include <stdint.h>

#define PAGE_SIZE 0x200000 // huge pages only
#define HIGHER_HALF 0xffff800000000000
#define KERNEL_OFFSET 0xffffffff80000000

#define PT_FLAG_PRESENT 1
#define PT_FLAG_WRITE 1 << 1
#define PT_FLAG_USER 1 << 2
#define PT_FLAG_NX 1 << 63
#define PT_FLAG_2MB 1 << 7

struct pm_chunk {
  uintptr_t base;
  size_t pages;
  struct pm_chunk *next;
  uint8_t bitmap[];
};

void pmm_set_page_used(uint8_t *bitmap_entry, int bit);
void pmm_set_page_free(uint8_t *bitmap_entry, int bit);

struct vm_chunk{
  uintptr_t base;
  size_t length;
  uint64_t flags;
  struct vm_chunk *next;
};
__attribute__((noreturn))
void init_mman();

#endif // !__MMAN_H__
