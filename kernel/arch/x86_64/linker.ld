OUTPUT_FORMAT(elf64-x86-64)

KERNEL_LMA = 0x0000000000200000; /* 2 MEG */
KERNEL_VMA = 0xffffffff80000000 + KERNEL_LMA;

ENTRY(bootstrap)

SECTIONS {
	. = KERNEL_VMA;

	_start = .;
	.text : AT(ADDR(.text) - KERNEL_VMA) {
		_code = .;
		*(.text*)
		. = ALIGN(0x1000);
	}

	.rodata : AT(ADDR(.rodata) - KERNEL_VMA) {
		. = ALIGN(0x1000);
	}

	.data : AT(ADDR(.data) - KERNEL_VMA) {
		_data = .;
		*(.data*)
		. = ALIGN(0x1000);
	}
	.bss : AT(ADDR(.bss) - KERNEL_VMA) {
		_bss = .;
		*(.bss*)
		*(COMMON)
		. = ALIGN(0x1000);
	}
	
	_end = .;

	/DISCARD/ : {
		*(.comment*)
		*(.note*)
	}
}
