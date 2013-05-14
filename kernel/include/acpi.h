#ifndef KERNEL_CPU_H
#define KERNEL_CPU_H

/*
 * kernel/src/include/acpi.h
 * ACPI Table Information
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

// ACPI Root System Descriptor Pointer (old version)
typedef struct AcpiRsdp
{
    char        signature[8];   // RSDP signature "RSD PTR "
    uint8_t     checksum;       // Checksum field
    char        oem[6];         // OEM Identifier
    uint8_t     revision;       // ACPI Revision
    uint32_t    rsdtAddr;       // Address of the RSDT

    uint32_t    length;         // Size of this table (rsdp)
    uint64_t    xsdtAddr;       // Address of the XSDT
    uint8_t     exChecksum;     // Extended checksum
    char        reserved[3];

} AcpiRsdp;

// Signature for the RSDP
#define ACPI_RSDP_SIG "RSD PTR "

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

// The ACPI Extended System Descriptor Table
typedef struct AcpiXsdt
{
    AcpiTableHeader header;     // Table header
    uint64_t        entry[];    // Table entries

} AcpiXsdt;

// The ACPI Multiple APIC Description Table
typedef struct AcpiMadt
{
    AcpiTableHeader header;         // Table header
    uint32_t        localApicAddr;  // Address of local APIC
    uint32_t        flags;          // APIC flags
    char            data[];         // MADT entries follow

} AcpiMadt;

// MADT entry types
#define ACPI_MADT_TAPIC     0
#define ACPI_MADT_TIOAPIC   1

// APIC flag which must be true to use that CPU
#define ACPI_MADT_APIC_EN   1

// Local APIC entry in the MADT
typedef struct AcpiMadtApic
{
    uint8_t         type;       // = 0
    uint8_t         length;     // = 8
    uint8_t         acpiId;     // ACPI Processor ID
    uint8_t         apicId;     // Local APIC ID
    uint32_t        flags;      // Processor Flags

} AcpiMadtApic;

// IO APIC entry in the MADT
typedef struct AcpiMadtIoApic
{
    uint8_t         type;       // = 1
    uint8_t         length;     // = 12
    uint8_t         acpiId;     // IO APIC ID
    uint8_t         reserved;
    uint32_t        addr;       // Address of this IO APIC
    uint32_t        intrBase;   // First interrupt number this IO APIC handles

} AcpiMadtIoApic;

// Verifies the checksum of an ACPI table
bool AcpiVerifyChecksum(void * data, uint32_t length);

// Finds and verifies the RSDP
//  Returns NULL if a valid one couldn't be found
AcpiRsdp * AcpiFindRsdp(void);

#endif
