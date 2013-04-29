#ifndef KERNEL_CONFIG_H
#define KERNEL_CONFIG_H

/*
 * kernel/include/config.h
 * Kernel configuration parameters
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

// The version of the L4 spec the kernel implements (X.2 Rev 7)
#define CONFIG_L4_VERSION   0x84070000

// L4 API flags
#define CONFIG_L4_FLAGS     0x00000008

// The L4 ID of this kernel (id 6, subid 1)
#define CONFIG_L4_ID        0x06010000

// Kernel version
//  High 8 bits = Major version
//  Next 8 bits = Minor version
//  Low 16 bits = Revision
#define CONFIG_VERSION      0x01000000

// The kernel supplier (only first 4 characters used)
#define CONFIG_SUPPLIER     "TheCow  "

// Extra version data (max 95 bytes)
#define CONFIG_VERSION_EXTRA    ("CowL4. James Cowgill. Built " __DATE__ ".")

// Timer interrupt rate
#define CONFIG_HZ           1000

#endif
