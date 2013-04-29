/*
 * kernel/src/kernel.ld
 * Kernel Linker Script
 *
 * Copyright (C) 2013 James Cowgill
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

ENTRY(BootStart)
OUTPUT_FORMAT(elf64-x86-64)

SECTIONS
{
    /* Kernel physical address start */
    MEM_BASE_VMA = 0xFFFFFFFF80000000;

    /* 32-bit boot code using physical addressing */
    . = 0x00100000;
    .boot32 ALIGN(0x1000) :
    {
        *(.boot32)
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
