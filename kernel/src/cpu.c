/*
 * kernel/src/cpu.c
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
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"
#include "atomic.h"
#include "cpu.h"
#include "cpupriv.h"
#include "kmemory.h"
#include "ioports.h"

// Global CPU variables
uint64_t CpuExternalBusFreq;
uint32_t CpuCount;
Cpu * CpuList[APIC_MAX_CPU];

// Address of the local apics
volatile uint8_t * CpuLocalApic;

// Initial value for the APIC timer
static uint32_t apicTimerInitial;

// Converts the high 8 bits of the apic id to a cpu id
uint32_t CpuApicToCpuId[APIC_MAX_CPU];

// Counter containing the number of up cpus
static volatile uint32_t initCpusUp;

// Verifies the checksum of an ACPI table
static bool AcpiVerifyChecksum(void * data, uint32_t length)
{
    uint8_t myChecksum = 0;

    // Add all the bytes up - result should be 0
    for (uint32_t i = 0; i < length; i++)
        myChecksum += ((char *) data)[i];

    return myChecksum == 0;
}

// Searches the region [start, end) for the rsdp structure
static AcpiRsdp * AcpiFindRsdpRange(uint32_t start, uint32_t end)
{
    // Get start and end pointers
    AcpiRsdp * currentPtr = KMemFromPhysical(start);
    AcpiRsdp * endPtr = KMemFromPhysical(end);

    // Do the search
    for (; currentPtr < endPtr; currentPtr++)
    {
        // Found it?
        if (memcmp(currentPtr->signature, "RSD PTR ", 8) == 0)
        {
            // Validate checksum
            if (!AcpiVerifyChecksum(currentPtr, 20))
                continue;

            // OK
            return currentPtr;
        }
    }

    // Didn't find it
    return NULL;
}

// Finds and verifies the MADT
//  Returns NULL if a valid one couldn't be found
static AcpiMadt * AcpiFindMadt(void)
{
    // Try to find the RSDP in EBDA and BIOS ROM
    AcpiRsdp * rsdp = AcpiFindRsdpRange(0x9FC00, 0x0A0000);

    if (!rsdp)
        rsdp = AcpiFindRsdpRange(0xE0000, 0x100000);

    if (rsdp && rsdp->rsdtAddr)
    {
        // Validate the RSDT
        AcpiRsdt * rsdt = KMemFromPhysical(rsdp->rsdtAddr);

        if (memcmp(rsdt->header.type, "RSDT", 4) == 0 &&
            AcpiVerifyChecksum(rsdt, rsdt->header.length))
        {
            // Search the RSDT for the MADT
            uint32_t entries = (rsdt->header.length - sizeof(AcpiTableHeader)) / 4;

            for (uint32_t i = 0; i < entries; i++)
            {
                AcpiMadt * madt = KMemFromPhysical(rsdt->entry[i]);

                if (memcmp(rsdt->header.type, "APIC", 4) == 0 &&
                    AcpiVerifyChecksum(madt, madt->header.length))
                {
                    // Good MADT
                    return madt;
                }
            }
        }
    }

    return NULL;
}

// Reads the given 32-bit APIC register
static uint32_t ApicRead32(uint16_t reg)
{
    return *((volatile uint32_t *) (CpuLocalApic + reg));
}

// Reads the given 32-bit APIC register
static void ApicWrite32(uint16_t reg, uint32_t value)
{
    *((volatile uint32_t *) (CpuLocalApic + reg)) = value;
}

// Adds a new CPU with the given APIC id
static void AddNewCpu(uint32_t apicId)
{
    Cpu * newCpu = KMemAllocate();

    // Initialize CPU structure
    newCpu->id = CpuCount;
    newCpu->apicId = apicId;

    newCpu->gdt[0] = 0;
    newCpu->gdt[1] = GDT_KERNEL_CODE;
    newCpu->gdt[2] = GDT_KERNEL_DATA;
    newCpu->gdt[3] = GDT_USER_DATA;
    newCpu->gdt[4] = GDT_USER_CODE;

    uint64_t tssAddr = (uint64_t) newCpu->tss;
    newCpu->gdt[5] = GDT_TSS | ((tssAddr & 0x00FFFFFF) << 16) | ((tssAddr & 0xFF000000) << 32);
    newCpu->gdt[6] = tssAddr >> 32;

    // Add to tables
    CpuApicToCpuId[apicId] = CpuCount;
    CpuList[CpuCount++] = newCpu;
}

// Populates the global CPU list with all the installed CPUs
static void PopulateCpuList(void)
{
    // Get the MADT from the ACPI tables
    AcpiMadt * madt = AcpiFindMadt();

    if (madt)
    {
        // Store apic address
        CpuLocalApic = KMemFromPhysical(madt->localApicAddr);

        // Parse the table
        uint32_t apicStructLen = madt->header.length - sizeof(AcpiMadt);

        for (uint32_t i = 0; i < apicStructLen; i += madt->data[i + 1])
        {
            // Must be an enabled processor
            if (madt->data[i] == ACPI_MADT_APIC && (madt->data[i + 4] & ACPI_MADT_APIC_EN))
            {
                // Add this cpu to the tables
                AddNewCpu(madt->data[i + 3]);
            }
        }

    }
    else
    {
        // No MADT - assume this is the only processor (handled below)
        CpuLocalApic = KMemFromPhysical(APIC_DEFAULT_ADDR);
    }

    // Ensure there's at least one processor
    if (CpuCount == 0)
        AddNewCpu(ApicRead32(APIC_REG_ID) >> 24);
}

// Calculates the settings for the APIC timer which
//  fire interrupts at the correct frequency
static void ApicCalibrateTimer(void)
{
    // Enable APIC timer
    ApicWrite32(APIC_REG_LVT_TIMER, APIC_INTR_TIMER);

    // Start PIT and count for 10ms
    uint8_t oldGatePort = IoInB(PIT_PORT_GATE) & 0xFC;
    IoOutB(PIT_PORT_GATE,    oldGatePort | 1);

    IoOutB(PIT_PORT_CONTROL, 0xB2);     // B2 = Channel 2 - One Shot
    IoOutB(PIT_PORT_CHAN2,   0x9B);     // 0x2E9B = counter value for 10ms
    IoOutB(PIT_PORT_CHAN2,   0x2E);

    // Pulse gate to reset counter
    IoOutB(PIT_PORT_GATE,    oldGatePort);
    IoOutB(PIT_PORT_GATE,    oldGatePort | 1);

    ApicWrite32(APIC_REG_TIME_INIT, 0xFFFFFFFF);

    // Wait until PIT is finished
    while ((IoInB(PIT_PORT_GATE) & 0x20) == 0)
        AtomicPause();

    // Disable APIC timer and get the counter value
    ApicWrite32(APIC_REG_LVT_TIMER, APIC_LVT_DISABLE);
    uint32_t apicCounterVal = ApicRead32(APIC_REG_TIME_CURR);

    // Calculate bus frequency in Hz (elapsed ticks * divide value * 1/10ms)
    CpuExternalBusFreq = (0xFFFFFFFF - apicCounterVal) * 2 * 100;

    // Calculate APIC initial value
    apicTimerInitial = CpuExternalBusFreq / CONFIG_HZ;
}

// Initializes the base registers of the local APIC (everything except timer)
static void ApicBaseInit(void)
{
    // APIC Configuration Registers
    ApicWrite32(APIC_REG_DFR,       0xFFFFFFFF);
    ApicWrite32(APIC_REG_LDR,       0);
    ApicWrite32(APIC_REG_TPR,       0);
    ApicWrite32(APIC_REG_TIME_DIV,  0);     // = divide by 2

    // Disable all local interrupts
    ApicWrite32(APIC_REG_LVT_CMCI,  APIC_LVT_DISABLE);
    ApicWrite32(APIC_REG_LVT_TIMER, APIC_LVT_DISABLE);
    ApicWrite32(APIC_REG_LVT_TSR,   APIC_LVT_DISABLE);
    ApicWrite32(APIC_REG_LVT_PERF,  APIC_LVT_DISABLE);
    ApicWrite32(APIC_REG_LVT_LINT0, APIC_LVT_DISABLE);
    ApicWrite32(APIC_REG_LVT_LINT1, APIC_LVT_DISABLE);
    ApicWrite32(APIC_REG_LVT_ERROR, APIC_LVT_DISABLE);

    // Enable APIC in the spurious interrupt vector
    ApicWrite32(APIC_REG_SPURIOUS,  APIC_INTR_SPURIOUS | 0x80);
}

// Initializes the APIC timer on this CPU
//  ApicCalibrateTimer must have been called before this
static void ApicTimerInit(void)
{
    ApicWrite32(APIC_REG_LVT_TIMER, APIC_INTR_TIMER | 0x20000);
    ApicWrite32(APIC_REG_TIME_INIT, apicTimerInitial);
}

// Sends an IPI to another processor
//  lowFields contains what type of IPI to send
static void ApicSendIpi(Cpu * dest, uint32_t lowFields)
{
    // Wait for previous IPI to complete
    while (ApicRead32(APIC_REG_INTR_CMD) & APIC_IPI_BUSY)
        AtomicPause();

    // Send this IPI
    ApicWrite32(APIC_REG_INTR_CMD + 0x10, dest->apicId << 24);
    ApicWrite32(APIC_REG_INTR_CMD       , lowFields);
}

Cpu * CpuCurrent(void)
{
    // Get APIC id and lookup in cpu list
    return CpuList[CpuApicToCpuId[ApicRead32(APIC_REG_ID) >> 24]];
}

// Performs the parts of CPU initialization which can be done later
static void CpuLateInit(Cpu * cpu)
{
    // Construct GDT pointer
    struct
    {
        uint8_t  pad[6];
        uint16_t size;
        void * ptr;

    } gdtPtr;

    gdtPtr.size = sizeof(cpu->gdt);
    gdtPtr.ptr  = cpu->gdt;

    // Run assembly part of initialization
    CpuLateInitAsm(&gdtPtr.size);

    // Start APIC timer
    ApicTimerInit();

    // Mark CPU as up
    AtomicAdd(&initCpusUp, 1);
}

// Entry point for non boot processors
void NO_RETURN CpuApEntry(Cpu * cpu)
{
    // Do late initialization
    CpuLateInit(cpu);

    // Just hang for the moment
#warning Todo go into some sort of idle loop here
    BREAKPOINT;
    for(;;)
        asm volatile ("hlt");
}

void CpuInitAll(void)
{
    Assert(CpuCount == 0);

    // Find all CPUs
    PopulateCpuList();

    // Initialize APIC
    ApicBaseInit();

    // Send an INIT to all other processors
    for (uint32_t i = 1; i < CpuCount; i++)
        ApicSendIpi(CpuList[i], APIC_IPI_INIT);

    // While the BIOS is initializing the other processors, we can calibrate the APIC timer
    ApicCalibrateTimer();

    // Copy lower memory code
    if (CpuCount > 1)
        memcpy(KMemFromPhysical(CPU_LOW_INIT_LOC), CpuLowerInit, CpuLowerInitEnd - CpuLowerInit);

    // Send a SIPI to all other processors
    for (uint32_t i = 1; i < CpuCount; i++)
        ApicSendIpi(CpuList[i], APIC_IPI_SIPI | (CPU_LOW_INIT_LOC >> 12));

    // Complete initialization of the boot processor
    CpuLateInit(CpuList[0]);

    // Wait for all other processors to complete
    while (initCpusUp < CpuCount)
        AtomicPause();
}
