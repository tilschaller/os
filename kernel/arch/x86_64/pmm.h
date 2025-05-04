#ifndef _PMM_H
#define _PMM_H

#include <stdint.h>

#define PMM_FREE 0
#define PMM_USED 1

typedef struct {
  uint64_t addr;
  uint64_t length;
  uint8_t bitmap[];
} pm_chunk;

void pm_chunk_create(const uint64_t base, const uint64_t length);

#endif // _PMM_H
