#define ASM_FILE
#include <kernel/multiboot2.h>

#define MBOOT2_ARCH 0
#define MBOOT2_LENGTH (multiboot_header_end - multiboot_header)
#define MBOOT2_CHECKSUM -(MULTIBOOT2_HEADER_MAGIC + MBOOT2_ARCH + MBOOT2_LENGTH)

.section .multiboot
multiboot_header:
        .int   MULTIBOOT2_HEADER_MAGIC
        .int   MBOOT2_ARCH
        .int   MBOOT2_LENGTH
        .int   MBOOT2_CHECKSUM
framebuffer_tag_start:  
        .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
        .short 0
        .int framebuffer_tag_end - framebuffer_tag_start
        .int 1024
        .int 768
        .int 32
framebuffer_tag_end:
        .int MULTIBOOT_HEADER_TAG_END
        .int 0
        .long 8
multiboot_header_end:
