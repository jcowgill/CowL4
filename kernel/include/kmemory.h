#ifndef KERNEL_KMEMORY_H
#define KERNEL_KMEMORY_H

/*
 * kernel/include/kmemory.h
 * Kernel-mode memory management
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
#include "multiboot.h"

// Initializes the kernel memory manager using the given region of memory
//  The base address is a virtual (not physical) address
//  The base address and length must be page aligned
void KMemInit(void * base, uint64_t length);

// Allocates 1 page of kernel memory
//  ZAllocate zeros the page before returning it
//  Returns NULL if out of memory
void * KMemAllocate(void);
void * KMemZAllocate(void);

// Frees 1 page of kernel memory
void KMemFree(void * page);

#endif
