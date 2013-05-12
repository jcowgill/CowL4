/*
 * kernel/src/acpi.c
 * ACPI Table Information
 *  This file only contains the small amount of ACPI code needed for the kernel.
 *  Most of ACPI is implemented as a user-mode service.
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
#include "acpi.h"

bool AcpiVerifyChecksum(void * data, uint32_t length)
{
    uint8_t myChecksum = 0;

    // Add all the bytes up - result should be 0
    for (uint32_t i = 0; i < length; i++)
        myChecksum += ((char *) data)[i];

    return myChecksum == 0;
}

// Searches the region [start, end) for the rsdp structure
static AcpiRsdp * AcpiFindRsdpRange(uint64_t start, uint64_t end)
{
    // Get start and end pointers
    AcpiRsdp * currentPtr = (AcpiRsdp *) start;
    AcpiRsdp * endPtr = (AcpiRsdp *) end;

    // Do the search
    for (; currentPtr < endPtr; currentPtr++)
    {
        // Found it?
        if (memcmp(currentPtr->signature, ACPI_RSDP_SIG, 8) == 0)
        {
            // Validate both checksums
            if (!AcpiVerifyChecksum(currentPtr, 20))
                continue;

            if (currentPtr->revision > 0 && !AcpiVerifyChecksum(currentPtr, sizeof(AcpiRsdp)))
                continue;

            // OK
            return currentPtr;
        }
    }

    // Didn't find it
    return NULL;
}

AcpiRsdp * AcpiFindRsdp(void)
{
    // Try EBDA and BIOS ROM areas
    AcpiRsdp * fpStruct;

    fpStruct = AcpiFindRsdpRange(0x9FC00, 0x0A0000);
    if (fpStruct) return fpStruct;

    fpStruct = AcpiFindRsdpRange(0xE0000, 0x100000);

    return fpStruct;
}
