ENTRY(_start)

SECTIONS
{
    . = 1M;

    .multiboot ALIGN(8) :
    {
        *(.multiboot)
    }

    .text :
    {
        *(.text)
    }

    .rodata :
    {
        *(.rodata)
    }

    .data :
    {
        *(.data)
    }

    .apps ALIGN(8) : SUBALIGN(8)
    {
        __app_start = .;
        KEEP(*(.apps))
        __app_end = .;
    }

    .bss :
    {
        *(COMMON)
        *(.bss)
    }
}