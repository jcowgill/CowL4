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
const IntrIdtPtrType IntrIdtPtr = { sizeof(IntrIdt) - 1, IntrIdt };

// CPU Exceptions
void IntrIsr0();
void IntrIsr1();
void IntrIsr3();
void IntrIsr6();
void IntrIsr7();
void IntrIsr8();
void IntrIsr10();
void IntrIsr11();
void IntrIsr12();
void IntrIsr13();
void IntrIsr14();
void IntrIsr16();
void IntrIsr17();
void IntrIsr18();
void IntrIsr19();

// APIC Interrupts
void IntrIsr32();
void IntrIsr33();

// Hardware Interrupts
void IntrIsr48();

// Fills in an IDT entry with the given ISR
static void FillIdtEntry(int index, void (* isr))
{
    // Fill constant fields
    IntrIdt[index].selector     = INTR_SELECTOR;
    IntrIdt[index].stackTable   = 0;
    IntrIdt[index].type         = INTR_TYPE_KERN;
    IntrIdt[index].reserved     = 0;

    // Fill entry pointer
    IntrIdt[index].ptr1         = (uintptr_t) isr;
    IntrIdt[index].ptr2         = (uintptr_t) isr >> 16;
    IntrIdt[index].ptr3         = (uintptr_t) isr >> 32;
}

void IntrInitIdt(void)
{
    // Fill CPU Exceptions
    FillIdtEntry( 0, IntrIsr0);
    FillIdtEntry( 1, IntrIsr1);
    FillIdtEntry( 3, IntrIsr3);
    FillIdtEntry( 6, IntrIsr6);
    FillIdtEntry( 7, IntrIsr7);
    FillIdtEntry( 8, IntrIsr8);
    FillIdtEntry(10, IntrIsr10);
    FillIdtEntry(11, IntrIsr11);
    FillIdtEntry(12, IntrIsr12);
    FillIdtEntry(13, IntrIsr13);
    FillIdtEntry(14, IntrIsr14);
    FillIdtEntry(16, IntrIsr16);
    FillIdtEntry(17, IntrIsr17);
    FillIdtEntry(18, IntrIsr18);
    FillIdtEntry(19, IntrIsr19);

    // Fill APIC Interrupts
    FillIdtEntry(32, IntrIsr32);
    FillIdtEntry(33, IntrIsr33);

    // Fill Hardware Interrupts
    for (int i = 0x30; i < 0x100; i++)
        FillIdtEntry(i, IntrIsr48);

    // Allow INT 3 (breakpoints) to be called from user mode
    IntrIdt[3].type = INTR_TYPE_USER;
}

void IntrInitPic(void)
{
    // Probably will implement the IO APIC at some point in the future
}

void IntrHandler(IntrContext context)
{
    /*
     * Exception Handling
     *
     * Special kernel handling
     * --------
     * 14 PF  Page Fault
     *  7 NM  Device not available (do FPU state switch)
     *
     * Causes panic (always)
     * --------
     *  1 DB Debug Exception
     *  8 DF Double Fault
     * 10 TS Invalid TSS
     * 18 MC Machine Check
     *
     * ALways handled by the thread's exception handler
     *  - Except when produced from kernel code (causes panic then)
     * --------
     *  0 DE Division By 0
     *  3 BP Breakpoint
     *  6 UD Invalid Opcode
     * 11 NP Segment Not Present
     * 12 SS Stack Fault
     * 13 GP General Protection
     * 16 MF x86 FPU Exception
     * 17 AC Alignment Check
     * 19 XM SIMD Exception
     */

    //
#warning TODO Interrupt handler
    BREAKPOINT;
}
