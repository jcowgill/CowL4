#ifndef KERNEL_GLOBAL_H
#define KERNEL_GLOBAL_H

/*
 * kernel/include/global.h
 * Global include file for the kernel
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

// External headers
#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// Global configuration
#include "config.h"

// Compiler attributes
#define ALIGN(x)    __attribute__((aligned(x)))
#define NO_RETURN   __attribute__((noreturn))

// Bochs breakpoints
#define BREAKPOINT  __asm volatile("xchgw %bx, %bx")

// Global functions
//  Panic - Immediately halts the kernel on all CPUs
void NO_RETURN Panic(const char * msg);

// memset - fills in a region of memory with the given value
void * memset(void * dest, uint8_t value, uint64_t len);

// memcpy - copies a region of memory (regions must not overlap)
void * memcpy(void * restrict dest, const void * restrict src, uint64_t len);

#endif
