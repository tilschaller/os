#include <stdint.h>
#include <string.h>

#include "pmm.h"

// base must be a valid virtual address
void pm_chunk_create(uint64_t base, uint64_t length) {
  // calculate the number of pages in this memory segment
  // divide by 0x1000, 4096
  int pages = length >> 12;

  // ensure pages is a multiple of eigth
  while (pages++%8 != 0) {}

  // create pm_chunk header at
  pm_chunk *chunk = (pm_chunk*)base;

  // set the usable addr of this chunk after the bitmap
  chunk->addr = base + pages / 8;
  chunk->length = length;

  // clear the pages in the bitmap
  memset((void*)base, PMM_FREE, pages / 8);
}
