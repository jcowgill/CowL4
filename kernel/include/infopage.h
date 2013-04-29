#ifndef KERNEL_INFOPAGE_H
#define KERNEL_INFOPAGE_H

/*
 * kernel/src/include/infopage.h
 * L4 Kernel Information Page
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
 * along with this program.  If not, see <http://  www.gnu.org/licenses/>.
 */

#include "global.h"

// The fields stored in the information page
//  All pointers are relative to the base address of this page
typedef struct
{
    char        magic[8];       // Magic number
    uint64_t    version;        // L4 Version
    uint64_t    flags;          // L4 flags
    uint64_t    kernDescPtr;    // Pointer to kernel description (immediately follows this field)

    uint64_t    kernId;         // Kernel ID
    uint64_t    kernDate;       // Kernel generation date (0 in this kernel)
    uint64_t    kernVersion;    // Kernel version
    char        kernSupplier[8];// Kernel supplier

    char        kernExtra[0x60];// Extra kernel info (must end with 2 null chars)

    uint64_t    clock;          // The value of the system clock
    uint32_t    memNumber;      // Number of memory descriptors
    uint32_t    memPtr;         // Pointer to first memory descriptor
    char        unused2[0x50];

    uint64_t    utcbInfo;       // Info about the UTCB structure
    uint64_t    kipSize;        // Size (log 2) of kernel information page

    char        unused3[0x10];
    uint64_t    bootInfo;       // Extra boot information
    uint64_t    procDescPtr;    // Pointer to list of processor descriptors

    uint64_t    clockPrecision; // Precision of the system clock
    uint64_t    threadInfo;     // Info about distribution of global thread ids
    uint64_t    pageInfo;       // Info about hardware page sizes
    uint64_t    procInfo;       // Info about number of processors

    uint64_t    pscSpaceControl;    // System call links
    uint64_t    pscThreadControl;
    uint64_t    pscProcessorControl;
    uint64_t    pscMemoryControl;
    uint64_t    scIpc;
    uint64_t    scLipc;
    uint64_t    scUnmap;
    uint64_t    scExchangeRegisters;
    uint64_t    scSystemClock;
    uint64_t    scThreadSwitch;
    uint64_t    scSchedule;
    uint64_t    unused4;

} InfoPageType;

// The global information page
extern InfoPageType InfoPage;

// Initializes the kernel info page
void InfoPageInit(void);

#endif
