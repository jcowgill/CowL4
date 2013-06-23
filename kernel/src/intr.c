/*
 * kernel/src/intr.c
 * Interrupt handling code
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
#include "intr.h"

// The global IDT
static IntrIdtEntry IntrIdt[256] ALIGN(4096);

// IDT Pointer
const IntrIdtPtrType IntrIdtPtr = { sizeof(IntrIdt) - 1, IntrIdt };

// CPU Exceptions
void IntrIsrIgnore();
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
    FillIdtEntry( 2, IntrIsrIgnore);
    FillIdtEntry( 3, IntrIsr3);
    FillIdtEntry( 4, IntrIsrIgnore);
    FillIdtEntry( 5, IntrIsrIgnore);
    FillIdtEntry( 6, IntrIsr6);
    FillIdtEntry( 7, IntrIsr7);
    FillIdtEntry( 8, IntrIsr8);
    FillIdtEntry( 9, IntrIsrIgnore);
    FillIdtEntry(10, IntrIsr10);
    FillIdtEntry(11, IntrIsr11);
    FillIdtEntry(12, IntrIsr12);
    FillIdtEntry(13, IntrIsr13);
    FillIdtEntry(14, IntrIsr14);
    FillIdtEntry(15, IntrIsrIgnore);
    FillIdtEntry(16, IntrIsr16);
    FillIdtEntry(17, IntrIsr17);
    FillIdtEntry(18, IntrIsr18);
    FillIdtEntry(19, IntrIsr19);
    FillIdtEntry(20, IntrIsrIgnore);

    // Fill APIC Interrupts
    FillIdtEntry(32, IntrIsrIgnore);
    FillIdtEntry(33, IntrIsr33);

    // Fill Hardware Interrupts
    for (int i = 0x30; i < 0x100; i++)
        FillIdtEntry(i, IntrIsr48);

    // Allow INT 3 (breakpoints) to be called from user mode
    IntrIdt[INTR_CPU_BP].type = INTR_TYPE_USER;
}

void IntrInitPic(void)
{
    // Probably will implement the IO APIC at some point in the future
}

void IntrHandler(IntrContext context)
{
    int intrNumber = context.intrNumber;

    // Which interrupt?
    switch (intrNumber)
    {
        // Interrupts which always cause a kernel panic
        case INTR_CPU_DB:
            Panic("Debug Exception");
            break;

        case INTR_CPU_DF:
            Panic("Double Fault");
            break;

        case INTR_CPU_TS:
            Panic("Invalid TSS");
            break;

        case INTR_CPU_MC:
            Panic("Machine Check Exception (buggy hardware)");
            break;

        default:
            Panic("Invalid Interrupt Number");
            break;

        // General CPU Exceptions
        case INTR_CPU_DE:
        case INTR_CPU_BP:
        case INTR_CPU_UD:
        case INTR_CPU_NP:
        case INTR_CPU_SS:
        case INTR_CPU_GP:
        case INTR_CPU_MF:
        case INTR_CPU_AC:
        case INTR_CPU_XM:
            // Panic if it occurred in the kernel
            if (context.cs == 0x08)
            {
                char errMsg[] = "Exception within the kernel:   ";

                // Fill exception number
                if (context.intrNumber >= 10)
                {
                    errMsg[sizeof(errMsg) - 3] = '1';
                    intrNumber -= 10;
                }

                errMsg[sizeof(errMsg) - 2] = '0' + intrNumber;

                // Panic
                Panic(errMsg);
            }

            // Raise user mode exception
#warning TODO Handle sending user mode exceptions
            break;

        // Special Exceptions
        case INTR_CPU_PF:
#warning TODO Handle Page Fault
            break;

        case INTR_CPU_NM:
#warning TODO Handle No Math Device
            break;

        // APIC Interrupts
        case INTR_APIC_TIMER:
#warning TODO Handle APIC Timer interrupt
            break;

        // Hardware Interrupts
        case INTR_IRQ:
        {
            // Get irq number
            int irqNumber = CpuGetHardwareIntr();

            if (irqNumber >= 0)
            {
#warning TODO Handle IRQ here
            }

            // Send EOI
            CpuSendEoi();
            break;
        }
    }
}
