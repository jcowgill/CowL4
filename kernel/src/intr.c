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
#include "ioports.h"
#include "intr.h"
#include "kmemory.h"

// IO APIC Information
typedef struct IntrIoApic
{
    // Base address (NULL = no io apic here)
    volatile uint32_t * baseAddr;

    // First IRQ and number of IRQs
    uint32_t baseIrq;
    uint32_t maxIrqs;

} IntrIoApic;

// IO APIC Storage
//  All IO APICs are stores from start to end with no gaps (of baseAddr == NULL)
static IntrIoApic IntrIoApicData[INTR_MAX_IOAPIC];

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

// Hardware Interrupts
void IntrIsr32();

// APIC Interrupts
void IntrIsr240();

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

    // Fill Hardware Interrupts
    for (int i = 0x20; i < 0xF0; i++)
        FillIdtEntry(i, IntrIsr32);

    // Fill APIC Interrupts
    FillIdtEntry(240, IntrIsr240);
    FillIdtEntry(255, IntrIsrIgnore);

    // Allow INT 3 (breakpoints) to be called from user mode
    IntrIdt[INTR_CPU_BP].type = INTR_TYPE_USER;

    // Disable legacy PIC
    IoOutB(0xA1, 0xFF);
    IoOutB(0x21, 0xFF);
}

// Reads an IO APIC register
static uint32_t IoApicRead32(volatile uint32_t * ioApicAddr, uint32_t reg)
{
    // Write reg to IOREGSEL and read IOWIN
    ioApicAddr[0] = reg;
    return ioApicAddr[2];
}

// Writes to an IO APIC register
static void IoApicWrite32(volatile uint32_t * ioApicAddr, uint32_t reg, uint32_t value)
{
    // Write reg to IOREGSEL and then write to IOWIN
    ioApicAddr[0] = reg;
    ioApicAddr[2] = value;
}

void IntrInitIoApic(uint32_t addr, uint32_t baseIrq)
{
    volatile uint32_t * ioApicAddr = KMemFromPhysical(addr);

    // Validate arguments
    if (addr == 0)
        Panic("IO APIC has null address in ACPI tables");

    uint32_t id  = IoApicRead32(ioApicAddr, INTR_IOAPIC_ID);
    uint32_t ver = IoApicRead32(ioApicAddr, INTR_IOAPIC_VER);

    // Bit of a hack to ensure this is an IO APIC
    if (id == ver)
        Panic("IO APIC doesn't work / exist");

    // Get number of IRQs on this IO APIC
    uint32_t maxIrqs = (ver >> 16) & 0xFF;

    // Find a slot for this IO APIC
    int ioApicIndex = 0;
    for (ioApicIndex = 0; ioApicIndex < INTR_MAX_IOAPIC; ioApicIndex++)
    {
        if (IntrIoApicData[ioApicIndex].baseAddr == NULL)
            break;
    }

    if (ioApicIndex == INTR_MAX_IOAPIC)
        Panic("Too many IO APICs");

    // Store data and initialize the IO APIC to default values
    IntrIoApicData[ioApicIndex].baseAddr = ioApicAddr;
    IntrIoApicData[ioApicIndex].baseIrq = baseIrq;
    IntrIoApicData[ioApicIndex].maxIrqs = maxIrqs;

    for (uint32_t i = 0; i < maxIrqs; i++)
    {
        // Write destination (always to CPU 0)
        IoApicWrite32(ioApicAddr, INTR_IOAPIC_TABLE + 2 * i + 1, 0);

        // Write flags + interrupt number
        uint32_t lowerFlags = (INTR_IRQ + baseIrq + i) | INTR_IOAPIC_MASKED;
        IoApicWrite32(ioApicAddr, INTR_IOAPIC_TABLE + 2 * i, lowerFlags);
    }
}

void IntrInitSetOverride(uint8_t isaIrq, uint32_t apicIrq, uint8_t flags)
{
    // Find the IO APIC containing isaIrq
    int ioApicIndex = 0;
    for (ioApicIndex = 0; ioApicIndex < INTR_MAX_IOAPIC; ioApicIndex++)
    {
        if (IntrIoApicData[ioApicIndex].baseAddr == NULL)
        {
            // Interrupt not found - ignore request
            return;
        }

        uint32_t upper = IntrIoApicData[ioApicIndex].baseIrq +
                         IntrIoApicData[ioApicIndex].maxIrqs;

        if (IntrIoApicData[ioApicIndex].baseIrq <= isaIrq && isaIrq < upper)
        {
            // IO APIC found
            break;
        }
    }

    // Exit if no IO APIC found
    if (ioApicIndex == INTR_MAX_IOAPIC)
        return;

    // Get old table flags
    volatile uint32_t * ioApicAddr = IntrIoApicData[ioApicIndex].baseAddr;
    uint32_t reg = INTR_IOAPIC_TABLE + 2 * (isaIrq - IntrIoApicData[ioApicIndex].baseIrq);

    uint32_t value = IoApicRead32(ioApicAddr, reg);

    // Overwrite new IRQ
    value = (value & 0xFFFFFF00) | (uint8_t) apicIrq;

    // Overwrite polarity and trigger mode
    if ((flags & 0x3) == 0x3)
        value |= INTR_IOAPIC_POL_LOW;
    else
        value &= ~INTR_IOAPIC_POL_LOW;

    if ((flags & 0xC) == 0xC)
        value |= INTR_IOAPIC_LEVEL;
    else
        value &= ~INTR_IOAPIC_LEVEL;

    // Rewrite IRQ register
    IoApicWrite32(ioApicAddr, reg, value);
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
