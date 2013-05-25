/*
 * kernel/src/intr.c
 * C entry points into the kernel
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
#include "intr.h"

// The global IDT
static IntrIdtEntry IntrIdt[256] ALIGN(4096);

// IDT Pointer
const IntrIdtPtrType IntrIdtPtr = { sizeof(IntrIdt), IntrIdt };

// ISR Entry Points
extern uint64_t IntrEntryStart[256];

void IntrInitIdt(void)
{
    // Fill in IDT
    for (int i = 0; i < 256; i++)
    {
        // Fill constant fields
        IntrIdt[i].selector     = INTR_SELECTOR;
        IntrIdt[i].stackTable   = 0;
        IntrIdt[i].type         = INTR_TYPE_KERN;
        IntrIdt[i].reserved     = 0;

        // Fill entry pointer
        uintptr_t isr = (uintptr_t) &IntrEntryStart[i];
        IntrIdt[i].ptr1         = isr;
        IntrIdt[i].ptr2         = isr >> 16;
        IntrIdt[i].ptr3         = isr >> 32;
    }

    // Allow INT 3 (breakpoints) to be called from user mode
    IntrIdt[3].type = INTR_TYPE_USER;
}

void IntrInitPic(void)
{
    // Probably will implement the IO APIC at some point in the future
}
