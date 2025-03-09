#ifndef __MMAN_H__
#define __MMAN_H__

#define PAGE_SIZE 0x200000 // huge pages only

__attribute__((noreturn))
void init_mman();

#endif // !__MMAN_H__
