#ifndef __MMAN_H__
#define __MMAN_H__

#define PAGE_SIZE 0x200000 // huge pages only
#define HIGHER_HALF 0xffff800000000000
#define KERNEL_OFFSET 0xffffffff80000000

#define PT_FLAG_PRESENT 1
#define PT_FLAG_WRITE 1 << 1
#define PT_FLAG_USER 1 << 2
#define PT_FLAG_NX 1 << 63
#define PT_FLAG_2MB 1 << 7

__attribute__((noreturn))
void init_mman();

#endif // !__MMAN_H__
