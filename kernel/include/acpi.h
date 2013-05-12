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

// Verifies the checksum of an ACPI table
bool AcpiVerifyChecksum(void * data, uint32_t length);

// Finds and verifies the RSDP
//  Returns NULL if a valid one couldn't be found
AcpiRsdp * AcpiFindRsdp(void);

#endif
