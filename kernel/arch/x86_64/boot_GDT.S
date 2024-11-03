#include <kernel/memory.h>

#define PRESENT         1 << 7
#define NOT_SYS         1 << 4
#define EXEC            1 << 3
#define DC              1 << 2
#define RW              1 << 1
#define ACCESSED        1 << 0

#define GRAN_4K         1 << 7
#define SZ_32           1 << 6
#define LONG_MODE       1 << 5

.intel_syntax noprefix
.section .rodata
.align PAGE_SIZE
.global BootGDT
.global BootGDTp
BootGDT:
  //Null Descriptor
  .long 0, 0
  //Code Descriptor
  .long 0xFFFF
  .byte 0
  .byte PRESENT | NOT_SYS | EXEC | RW
  .byte GRAN_4K | LONG_MODE | 0xF 
  .byte 0

BootGDTp:
  .short 2*8-1
  .quad offset BootGDT
