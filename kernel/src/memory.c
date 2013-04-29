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
//  1000 - 1FFF = PDPT (covers first and last 512 GB)
//  2000 - 2FFF = First PDT (covers first 1GB of physical memory)
//  3000 - 3FFF = Second PDT (covers second 1GB of physical memory)
uint8_t MemKernelTables[0x4000] ALIGN(4096);
