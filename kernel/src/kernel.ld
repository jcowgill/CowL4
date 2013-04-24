/* Kernel Linker Script */

ENTRY(Init32)
OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
    /* Kernel physical address start */
    MEM_BASE_VMA = 0xFFFFFFFF80000000;

    /* 32-bit init code using physical addressing */
    . = 0x0010000;
    .init32 ALIGN(0x1000) :
    {
        *(.init32)
    }

    /* Move to high addresses */
    . += MEM_BASE_VMA;

    /* Kernel code */
    .text ALIGN(0x1000) : AT(ADDR(.text) - MEM_BASE_VMA)
    {
        *(.text)
    }

    /* Kernel read only data */
    .rodata ALIGN(0x1000) : AT(ADDR(.rodata) - MEM_BASE_VMA)
    {
        *(.rodata)
    }

    /* Kernel data */
    .data ALIGN(0x1000) : AT(ADDR(.data) - MEM_BASE_VMA)
    {
        *(.data*)
        *(.gnu.linkonce.d*)
    }

    /* Kernel bss */
    .bss ALIGN(0x1000) : AT(ADDR(.bss) - MEM_BASE_VMA)
    {
       *(COMMON*)
       *(.bss)
    }
}
