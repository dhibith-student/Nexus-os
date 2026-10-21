ENTRY(_start)
SECTIONS
{
    . = 0xffffffff80000000;
    __kernel_image_start = .;
    .limine_requests : ALIGN(0x1000) {
        KEEP(*(.limine_requests_start))
        KEEP(*(.limine_requests))
        KEEP(*(.limine_requests_end))
    }
    .text : ALIGN(0x1000) { KEEP(*(.text.boot)) *(.text .text.*) }
    .rodata : ALIGN(0x1000) { *(.rodata .rodata.*) }
    .data : ALIGN(0x1000) { *(.data .data.*) }
    .bss : ALIGN(0x1000) { *(COMMON) *(.bss .bss.*) }
    __kernel_image_end = .;
    /DISCARD/ : { *(.eh_frame*) *(.note*) }
}
