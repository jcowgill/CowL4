#ifndef KERNEL_CPUPRIV_H
#define KERNEL_CPUPRIV_H

/*
 * kernel/src/include/cpupriv.h
 * Private headers for the CPU (mainly to remove structures from cpu.c)
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

// MADT entry types
#define ACPI_MADT_APIC          0
#define ACPI_MADT_IOAPIC        1
#define ACPI_MADT_IRQOVERRIDE   2

// APIC flag which must be true to use that CPU
#define ACPI_MADT_APIC_EN   1

// ACPI Root System Descriptor Pointer (old version)
typedef struct AcpiRsdp
{
    char        signature[8];   // RSDP signature "RSD PTR "
    uint8_t     checksum;       // Checksum field
    char        oem[6];         // OEM Identifier
    uint8_t     revision;       // ACPI Revision
    uint32_t    rsdtAddr;       // Address of the RSDT

} AcpiRsdp;

// The header of all ACPI tables
typedef struct AcpiTableHeader
{
    char        type[4];        // Table signature
    uint32_t    length;         // Table size
    uint8_t     revision;       // Table revision
    uint8_t     checksum;       // Table checksum field
    char        oem[6];         // OEM Identifier
    char        oemTableId[8];  // OEM Table Identifier
    uint32_t    oemRevision;    // OEM Revision
    char        creator[4];     // Creator Identifier
    uint32_t    creatorRevision;// Creator Revision

} AcpiTableHeader;

// The ACPI Root System Descriptor Table
typedef struct AcpiRsdt
{
    AcpiTableHeader header;     // Table header
    uint32_t        entry[];    // Table entries

} AcpiRsdt;

// The ACPI Multiple APIC Description Table
typedef struct AcpiMadt
{
    AcpiTableHeader header;         // Table header
    uint32_t        localApicAddr;  // Address of local APIC
    uint32_t        flags;          // APIC flags
    uint8_t         data[];         // MADT entries follow

} AcpiMadt;

// Local APIC entry in the MADT
typedef struct AcpiMadtApic
{
    uint8_t         type;       // = 0
    uint8_t         length;     // = 8
    uint8_t         acpiId;     // ACPI Processor ID
    uint8_t         apicId;     // Local APIC ID
    uint32_t        flags;      // Processor Flags

} AcpiMadtApic;

// GDT descriptor templates
#define GDT_KERNEL_CODE      (0x00A09BUL << 40)
#define GDT_KERNEL_DATA      (0x00C093UL << 40)
#define GDT_USER_CODE        (0x00A0FBUL << 40)
#define GDT_USER_DATA        (0x00C0F3UL << 40)
#define GDT_TSS             ((0x000089UL << 40) | 0x68)

// Maximum addressable cpus (xAPIC)
#define APIC_MAX_CPU        256

// Default APIC location
#define APIC_DEFAULT_ADDR   0xFEE00000

// Local APIC register indexes
#define APIC_REG_ID         0x0020  // Local APIC ID
#define APIC_REG_VERSION    0x0030  // APIC Version

#define APIC_REG_TPR        0x0080  // Task Priority
#define APIC_REG_APR        0x0090  // Arbitration Priority
#define APIC_REG_PPR        0x00A0  // Processor Priority
#define APIC_REG_EOI        0x00B0  // End of interrupt
#define APIC_REG_RRD        0x00C0  // Remote Read
#define APIC_REG_LDR        0x00D0  // Local destination
#define APIC_REG_DFR        0x00E0  // Destination format
#define APIC_REG_SPURIOUS   0x00F0  // Spurious interrupt vector

#define APIC_REG_ISR        0x0100  // In service register (8 regs long)
#define APIC_REG_TMR        0x0180  // Trigger mode register (8 regs long)
#define APIC_REG_IRR        0x0200  // Interrupt request register (8 regs long)

#define APIC_REG_ERROR      0x0280  // Error status register

#define APIC_REG_LVT_CMCI   0x02F0  // LVT CMCI
#define APIC_REG_INTR_CMD   0x0300  // Interrupt command register (2 regs long)
#define APIC_REG_LVT_TIMER  0x0320  // LVT Timer
#define APIC_REG_LVT_TSR    0x0330  // LVT Thermal Sensor
#define APIC_REG_LVT_PERF   0x0340  // LVT Performance Counters
#define APIC_REG_LVT_LINT0  0x0350  // LVT LINT0
#define APIC_REG_LVT_LINT1  0x0360  // LVT LINT1
#define APIC_REG_LVT_ERROR  0x0370  // LVT Error register

#define APIC_REG_TIME_INIT  0x0380  // Initial Timer Count
#define APIC_REG_TIME_CURR  0x0390  // Current Timer Count
#define APIC_REG_TIME_DIV   0x03E0  // Timer Divide Register

// Other APIC constants
#define APIC_LVT_DISABLE    0x10000 // Disables an LVT interrupt

#define APIC_IPI_INIT       0x4500  // Low fields for an init ipi
#define APIC_IPI_SIPI       0x4600  // Low fields for an startup ipi (except vector)

#define APIC_IPI_BUSY       0x1000  // Bit set if APIC is sending an ipi

// PIT constants
#define PIT_PORT_CONTROL    0x43    // PIT Control Port
#define PIT_PORT_CHAN2      0x42    // PIT Channel 2 Data Port
#define PIT_PORT_GATE       0x61    // Gate register controlling PIT channel 2

// Temp Init location
#define CPU_LOW_INIT_LOC    0x8000  // Physical location to place the lower memory init code

// Local APIC address and APIC conversion (used to get cpu id by cpu init code)
extern volatile uint8_t * CpuLocalApic;
extern uint32_t CpuApicToCpuId[APIC_MAX_CPU];

// Start and end points for lower cpu code
extern char CpuLowerInit[1];
extern char CpuLowerInitEnd[1];

// C entry point for non boot processors
void NO_RETURN CpuApEntry(Cpu * cpu);

// Assembly part of late initialization
void CpuLateInitAsm(void * gdtPtr);

// Reads the given 32-bit APIC register
static inline uint32_t ApicRead32(uint16_t reg)
{
    return *((volatile uint32_t *) (CpuLocalApic + reg));
}

// Reads the given 32-bit APIC register
static inline void ApicWrite32(uint16_t reg, uint32_t value)
{
    *((volatile uint32_t *) (CpuLocalApic + reg)) = value;
}

#endif
