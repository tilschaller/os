#define ASM

#include "boot.h"

.globl  kernel_main

.code32
.globl bootstrap, init_pml4, init_pdp, init_pd
.section .text
bootstrap:
	/* make sure interrupts are off */
	cli

	/* load the GDT */
	lgdt (init_gdt64_ptr - KERNEL_VMA)

	/* give ourselves a functional stack */
	movl $(init_stack_end - KERNEL_VMA), %esp

	/* reset EFLAGS. */
	pushl $0
	popf

	pushl %eax
	pushl %ebx

	/* zero out bss section */
	xorl    %eax, %eax
	movl    $(_bss - KERNEL_VMA), %edi
	movl    $(_end - KERNEL_VMA), %ecx
	subl    %edi, %ecx
	cld
	rep	stosb

	/* get the multiboot info back */
	popl %esi
	popl %edi

	/* ensure that paging is off */
	movl    %cr0, %eax
	andl    $0x7fffffff, %eax
	movl    %eax, %cr0

	/* enable CR4.PAE */
	movl    %cr4, %eax
	orl     $0x20, %eax
	movl    %eax, %cr4

	/* load a level4 PD */
	movl    $(init_pml4 - KERNEL_VMA), %eax
	mov     %eax, %cr3

	/* set IA32_EFER.LME */
	movl    $0xc0000080, %ecx
	rdmsr
	/* set the PAE and SYSENTER bits */
	orl     $0x00000101, %eax
	wrmsr

	/* enable paging */
	movl    %cr0, %eax
	orl     $0x80000000, %eax
	movl    %eax, %cr0

	/* at this point we should be in IA-32e mode */
	ljmp $0x08, $(boot64 - KERNEL_VMA)

.data
.align 16
gdt64:
	.quad 0x0000000000000000 // 0x00 NULL
	.quad 0x0020980000000000 // 0x08 KCODE64
	.quad 0x0000920000000000 // 0x10 KDATA64
gdt64_end:

.align 16
init_gdt64_ptr:
	.word gdt64_end - gdt64 - 1
	.quad gdt64 - KERNEL_VMA

.align 16
init_gdt64_ptr_high:
    .word gdt64_end - gdt64 - 1
    .quad gdt64

/* ident map of first 4MB GB, and mirror it at the -2GB mark  */
.align 0x1000
init_pml4:
	.quad init_pdp - KERNEL_VMA + 3 // [0x0000000000000000 - 0x00000007ffffffff)
	.fill 510,8,0
	.quad init_pdp - KERNEL_VMA + 3 // [0xfffffff800000000 - 0xffffffffffffffff)

init_pdp:
	.quad init_pd - KERNEL_VMA + 3
	.fill 509,8,0
	.quad init_pd - KERNEL_VMA + 3
	.fill 1,8,0

init_pd:
	.quad 0x0000000000000083 // 0MB - 2MB
	.quad 0x0000000000200083 // 2MB - 4MB
	.fill 510,8,0


init_stack_start:
	.fill 0x1000,1,0
init_stack_end:


.code64
boot64_high:
	/* adjust the stack pointer to be in upper memory */
	movq $KERNEL_VMA, %rax
	addq %rax, %rsp

	/* unmap the kernel from low memory */
	movq   $0x0,init_pml4
	movq   $0x0,init_pdp
	invlpg 0

	/* go to our C entry point */
	call kernel_main

boot64_hang:
	hlt
	jmp boot64_hang

boot64:
    /* update the GDT */
    lgdt init_gdt64_ptr_high

    mov $0x10, %rax
    mov %rax, %ds
    mov %rax, %ss
    mov %rax, %es
    mov %rax, %gs
    mov %rax, %fs

	/* do an indirect to force non-relative jmp */
	movabsq $boot64_high, %rax
	jmp *%rax
