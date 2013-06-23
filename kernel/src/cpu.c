/*
 * kernel/src/cpu.c
 * CPU Information and Management
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
#include "intr.h"

// Global CPU variables
uint64_t CpuExternalBusFreq;
uint32_t CpuCount;
Cpu * CpuList[APIC_MAX_CPU];

// Address of the local apics
volatile uint8_t * CpuLocalApic;

// Converts the high 8 bits of the apic id to a cpu id
uint32_t CpuApicToCpuId[APIC_MAX_CPU];

void CpuSendIpi(Cpu * dest, uint32_t lowFields)
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

int CpuGetHardwareIntr(void)
{
    // Search the ISR for the highest priority interrupt
    for (uint16_t reg = APIC_REG_ISR + 0x70; reg >= APIC_REG_ISR; reg -= 0x10)
    {
        uint32_t isrValue = ApicRead32(reg);

        if (isrValue != 0)
        {
            // Calculate and return irq number
            int highestBit = (sizeof(long) * 8) - CLZ(isrValue) - 1;
            int regOffset  = (reg - APIC_REG_ISR) * 2;

            return highestBit + regOffset - INTR_IRQ;
        }
    }

    // No interrupts found
    return -1;
}

void CpuSendEoi(void)
{
    ApicWrite32(APIC_REG_EOI, 1);
}
