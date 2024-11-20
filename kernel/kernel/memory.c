#include <kernel/memory.h>
#include <kernel/multiboot2.h>
#include <stdint.h>
#include <string.h>

extern const void kernel_end;
extern struct multiboot_tag_mmap *memmap;

struct pm_chunk {
  struct pm_chunk *prev;
  struct pm_chunk *next;
  uint64_t len;
  void *addr;
};

// TODO: actually make the mem of this chunk usable again
// move all behind the to be removed one forward: memmove
void remove_pm_chunk(struct pm_chunk *pm_chunk) {}

// only join chunk next to each other in mem
// remove one make the other to both
void join_pm_chunk(struct pm_chunk *pm_chunk) {}

// pass the chunk behind which you want the chunk
void add_pm_chunk(struct pm_chunk *pm_chunk) {}

struct pm_chunk pm_free_s = {NULL, NULL, 0, NULL};
struct pm_chunk pm_used_s = {NULL, NULL, 0, NULL};

void init_mman() {
  int num_entries = (memmap->size - 8) / memmap->entry_size;
  struct multiboot_mmap_entry entries[num_entries];

  memcpy(&entries, &memmap->entries, memmap->size - 8);

  struct pm_chunk *pm_free = &pm_free_s;
  void *prev_addr;

  for (int i = 0; i < num_entries; i++) {
    int j = 0;

    if (entries[i].type == 1) {
      while (pm_free->len != 0) {
        pm_free = pm_free->next;
        j++;
      }
      pm_free->addr = (void *)entries[i].addr;
      pm_free->len = entries[i].len;
      pm_free->next = ((struct pm_chunk *)&kernel_end) + j;
      if (j != 0) {
        pm_free->prev = prev_addr;
      }
      prev_addr = pm_free;
    }
  }

  pm_free->next = NULL;

  // TODO: map first two mb for kernel

  return;
}

void *pmm_chunk(uint64_t size) {
  struct pm_chunk *pm_free = &pm_free_s;
  struct pm_chunk *pm_used = &pm_used_s;

  while (pm_free->len < size) {
    if (pm_free->next == NULL) {
      return NULL;
    }
    pm_free = pm_free->next;
  }

  void *return_addr = pm_free->addr;

  pm_free->len = pm_free->len - size;
  pm_free->addr = pm_free->addr + size;

  // TOOD: if chunk is empty remove
  if (pm_free->addr == 0) {
  }

  // TODO: add used chunk to pm_used list

  return return_addr;
}

void pmm_chunk_free() {}

void *mmap(size_t size) {}

void mmap_free() {}
