/*
 * kernel/include/time.h
 * Timimg and time period functions
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
#include "time.h"

TimePeriod TimeMakePeriod(uint64_t microSeconds)
{
    // Handle zero
    if (microSeconds == 0)
        return TIME_PEROID_ZERO;

    // Calculate log2 of microSeconds
    uint32_t log2 = (uint32_t) (63 - CLZ(microSeconds));

    // What type of value to produce?
    if (log2 < 10)
        return (TimePeriod) microSeconds;   // Denormal
    else if (log2 > 40)
        return TIME_PEROID_INFINITE;        // Infinite (too large)
    else
        return (TimePeriod) (((log2 - 9) << 10) | (microSeconds >> (log2 - 9)));
}

uint64_t TimeExpandPeriodBase(TimePeriod period, uint64_t base)
{
    uint64_t mantissa = period & 0x3FF;

    if ((period & 0x8000) == 0)
    {
        // Relative period (these ADD to the base)
        return base + (mantissa << (period >> 10));
    }
    else
    {
        // Absolute period (these REPLACE the lower bits of the base)
        //  I'm not sure if this is right, the spec isn't very clear
        bool carry = period & 0x0400;
        uint8_t exponent = (period & 0x7FFF) >> 11;

        // Get unshifted part
        uint64_t baseShiftExp = base >> exponent;
        uint64_t unshifted = mantissa + (baseShiftExp & ~0x3FFU);

        // Check carry flag
        if (carry != (baseShiftExp & 0x400))
            unshifted += 0x400;

        // Return final result
        return unshifted << exponent;
    }
}

uint64_t TimeExpandPeriod(TimePeriod period)
{
    return TimeExpandPeriodBase(period, InfoPage.clock);
}
