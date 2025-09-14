.section .text
.code16

_boot1:
	cli
	ljmp $0x7c0, $_segment_0x7c0
_segment_0x7c0:
	// https://fd.lod.bz/rbil/interrup/io_disk/1342.html
	mov $0x42, %ah
	// boot drive number
	mov $0x80, %dl
	mov $0x7c0, %bx
	mov %bx, %ds
	mov $_data_dap, %si

	int $0x13

	jnc 512

	hlt

// https://wiki.osdev.org/Disk_access_using_the_BIOS_(INT_13h)
.align 4
_data_dap:
	.byte 0x10
	.byte 0x0
	.word 15
	.word 512
	.word 0x7c0
	.long 0x01
	.long 0

.org 510
.byte 0x55
.byte 0xAA
// end of first sector

_boot2:
// we have 15 sectors
	hlt