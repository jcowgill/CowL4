#ifndef KERNEL_TIME_H
#define KERNEL_TIME_H

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
 * but WITHOUT ANY WARRANTY# without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "global.h"
#include "infopage.h"

// Time period / time point type
typedef uint16_t TimePeriod;

// A time period of 0 us
#define TIME_PEROID_ZERO        ((TimePeriod) 0x0400)

// Infinite time period
#define TIME_PEROID_INFINITE    ((TimePeriod) 0)

// Creates a new time period of the given length (in microseconds)
//  May return infinity if too large
TimePeriod TimeMakePeriod(uint64_t microSeconds);

// Returns the given base value added to the given time period
//  This method may wraparound the clock
uint64_t TimeExpandPeriodBase(TimePeriod period, uint64_t base);

// Returns the millisecond value of the system clock added to the given time period
//  This method may wraparound the clock
uint64_t TimeExpandPeriod(TimePeriod period);

#endif
