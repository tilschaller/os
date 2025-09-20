OUTPUT_FORMAT(binary)
STARTUP(arch/x86_64/boot.o)

MEMORY {
    RAM (wx) : ORIGIN = 0x7c00, LENGTH = 256k
}

SECTIONS {
    .text : 
    {
        *(.text*)
    } > RAM
}