.section .text
.code16

_boot1:
	cli
	ljmp $0x0, $_segment_0
	
_segment_0:
	// https://fd.lod.bz/rbil/interrup/io_disk/1342.html
	mov $0x42, %ah
	// boot drive number
	mov $0x80, %dl
	mov $0x0, %bx
	mov %bx, %ds
	mov $_data_dap, %si

	int $0x13

	// if we succesfully loaded the 10 sectors into RAM we jump to the newly loaded sector
	jnc _boot2

	hlt

// https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)
.align 4
_data_dap:
	.byte 0x10
	.byte 0x0
	.word 9
	.word 512
	.word 0x7c0
	.long 1
	.long 0

.org 510
.byte 0x55
.byte 0xAA
// end of first sector
.org 512
_boot2:
	// we have 10 sectors (size of unused bytes on iso filesystem)
	// enabling a20 line using bios (we just hope this is supported)
	mov $0x2403, %ax
	int $0x15 // testing if a20 is already on
	jb	hlt // int failed
	cmp $0x0, %ah 
	jnz hlt // int failes
	cmp $1, %al 
	jz _a20_activated // already on
	// else we enable
	// activating a20
	mov $0x2401, %ax
	int $0x15
	jb hlt // enabling failed
	cmp $0x0, %ah 
	jnz hlt // enabling failed
	// move on with a20 enabled
	jmp _a20_activated

_a20_activated:
	// fetching a mmap using int 15h, eax=0xe820
	mov $0x0, %ax 
	mov %ax, %es 
	mov $0x7c04, %di 
	xor %ebx, %ebx 
	xor %bp, %bp 
	mov $0x0534D4150, %edx
	mov $0xe820, %eax 
	movl $1, %es:20(%di)
	mov 24, %ecx 
	int $0x15
	jc hlt
	mov $0x0534D4150, %edx 
	cmp %edx, %eax
	jne hlt 
	test %ebx, %ebx 
	je hlt 
	jmp .jmpin
.e820lp:
	mov $0xe820, %eax
	movl $1, %es:20(%di)
	mov $0x24, %ecx 
	int $0x15
	jc .e820f 
	mov $0x0534D4150, %edx
.jmpin:
	jcxz .skipent
	cmp $20, %cl 
	jbe .notext 
	testb $1, %es:20(%di) 
	je .skipent
.notext:
	mov %es:8(%di), %ecx
	or %es:12(%di), %ecx 
	jz .skipent
	inc %bp 
	add $24, %di
.skipent:
	test %ebx, %ebx
	jne .e820lp
.e820f:
	mov $0x7c00, %si
	movw %bp, %es:(%si)
	clc

	// memory map is done, find the kernel
	// it is directly after our 10 sectors
	// first map the elf header onto sector 11
	mov $0x42, %ah
	// boot drive number
	mov $0x80, %dl
	mov $0x0, %bx
	mov %bx, %ds
	mov $_data_dap_elf_header, %si

	int $0x13
	jc hlt

	mov $0x900, %bx
	mov %bx, %ds
	xor %si, %si 
	movb %ds:(%si), %al
	cmp $0x7f, %al
	jne hlt

	// move lower half of phoff into ax
	// it should be that large anyway
	movw %ds:32(%si), %ax

	add %ax, %si
	// if first program header is not PT_LOAD we just hlt
	cmpb $1, %ds:(%si)
	jne hlt

	mov $0x0, %ax 
	mov %ax, %fs 
	mov $_data_dap_elf_load_program, %di

	movl %ds:8(%si), %eax
	// divide by 512
	shr $9, %eax
	// add 10 to get offset from image
	add $10, %eax
	// write eax into the dap
	movl %eax, %fs:8(%di)
	
	// same for file size
	movl %ds:32(%si), %eax
	shr $9, %eax
	movw %ax, %fs:2(%di)

	// load the kernel
	mov $0x42, %ah
	mov $0x80, %dl
	mov $0x0, %bx
	mov %bx, %ds
	mov $_data_dap_elf_load_program, %si
	int $0x13
	jc hlt

	// activating protected mode
	lgdt _data_gdtr

	mov %cr0, %eax
	or $1, %al
	mov %eax, %cr0

	ljmp $0x8, $_protected_mode

hlt:
	mov $0x87654321, %eax
	hlt

.code32
_protected_mode:
	mov $0x10, %ax
	mov %ax, %ds 
	mov %ax, %es 
	mov %ax, %fs 
	mov %ax, %gs
	mov %ax, %ss 

	movw %fs:2(%di), %cx
	shl $9, %ecx

	mov $0x9200, %esi // kernel source pointer
	mov $0x200000, %edi // dest pointer

	cld

	rep movsb

	mov $0x12345678, %eax

	jmp 0x200000

	hlt

.align 16
_data_gdt:
	.quad 0x0000000000000000 // 0x00 
	.quad 0x00cf9a000000ffff // 0x08
	.quad 0x00cf92000000ffff // 0x10
_data_gdt_end:
_data_gdtr:
	.word _data_gdt_end - _data_gdt - 1
	.long _data_gdt

.align 4
_data_dap_elf_header:
	.byte 0x10
	.byte 0x0
	.word 1
	.word 0
	.word 0x900
	.long 10
	.long 0

.align 4
_data_dap_elf_load_program:
	.byte 0x10
	.byte 0x0
	.word 0x0 // we write this
	.word 0x0
	.word 0x920
	.long 0x0 // we write this
	.long 0

// just to check if its too large
.org (512 * 10)