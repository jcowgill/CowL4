#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

/*
 * kernel/src/include/cpu.h
 * CPU Information and Initialization
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

// Information about a cpu and per-cpu fields
typedef struct Cpu
{
    uint32_t id;            // Logical ID of this cpu (= index in CpuList)
    uint8_t  apicId;        // ID of the cpu's local APIC

    uint64_t gdt[7];        // The GDT for this CPU
    uint32_t tss[0x68];     // The TSS for this CPU

} Cpu;

// List of all the cpus in the system
extern uint32_t CpuCount;
extern Cpu * CpuList[];

// External bus frequency
extern uint64_t CpuExternalBusFreq;

// Returns the current cpu's structure
Cpu * CpuCurrent(void);

// Sends an IPI to another processor
//  lowFields contains what type of IPI to send
void CpuSendIpi(Cpu * dest, uint32_t lowFields);

// Initializes all the CPUs (including the calling one) on the system
void CpuInitAll(void);

#endif
