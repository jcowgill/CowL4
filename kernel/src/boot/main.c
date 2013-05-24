/*
 * kernel/src/boot/main.c
 * Main Kernel Booting Code
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
#include "cpu.h"
#include "kmemory.h"
#include "multiboot.h"

void NO_RETURN BootMain(MultibootInfo * bootInfo);

void NO_RETURN BootMain(MultibootInfo * bootInfo)
{
#warning Todo - use the memory map for this
    // Setup the kernel memory manager (currently 2MB starting at 2MB address)
    (void) bootInfo;
    KMemInit(0x00200000, 0x00200000);

#warning Initialize interrupts before cpus

    // Initialize all CPUs
    CpuInitAll();

#warning Enable global bit on all pages here

    Panic("Nothing here yet");
}
