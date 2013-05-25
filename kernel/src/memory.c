/*
 * kernel/src/memory.c
 * General Kernel Memory Management
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

#include "global.h"

// Kernel only paging tables
//  Initialized in start.s
//  0000 - 0FFF = PML4 Table
//  1000 - 1FFF = PDPT (512 GB mapped pages)
//  2000 - 5FFF = 4 PDT Tables (covering first 4GB of physical memory)
uint8_t MemKernelTables[0x6000] ALIGN(4096);

void MemEnableGlobalPages(void)
{
    // Unmap lower identity mapped region
    //  This will be flushed the first time a processor switches to another address space
    MemKernelTables[0] = 0;
    MemKernelTables[1] = 0;
    MemKernelTables[2] = 0;
    MemKernelTables[3] = 0;

    // Enable global bit in all PDT entries
    for (int i = 0x2001; i < 0x6000; i += 8)
        MemKernelTables[i] |= 1;        // Global flag
}
