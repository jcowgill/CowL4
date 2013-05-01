/*
 * kernel/src/kmemory.c
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
#include "kmemory.h"
#include "atomic.h"

// Root of the memory stack
static void * KMemStackRoot;

// Spinlock for the stack root
static AtomicSpinlock KMemStackLock;

void * KMemAllocate(void)
{
    void * newPage;

    // Enter lock
    AtomicLock(&KMemStackLock);
    {
        // Pop one item off the stack
        newPage = KMemStackRoot;

        if (newPage != NULL)
            KMemStackRoot = *((void **) KMemStackRoot);
    }
    AtomicUnlock(&KMemStackLock);

    return newPage;
}

void * KMemZAllocate(void)
{
    // Allocate memory and zero it
    void * newPage = KMemAllocate();

    if (newPage != NULL)
        memset(newPage, 0, 4096);

    return newPage;
}

void KMemFree(void * page)
{
    // Ignore NULL free
    if (page == NULL)
        return;

    // Enter lock
    AtomicLock(&KMemStackLock);
    {
        // Push this page onto the stack
        *((void **) page) = KMemStackRoot;
        KMemStackRoot = page;
    }
    AtomicUnlock(&KMemStackLock);
}

void KMemInit(void * base, uint64_t length)
{
    uint64_t rawBase = (uint64_t) base;

    // Require page alignment and some memory
    Assert(((uint64_t) base & 0xFFF) == 0);
    Assert((length & 0xFFF) == 0);
    Assert(length > 0);
    
    // Push all memory onto the stack
    for (uint64_t i = 0x1000; i < length; i += 0x1000)
        *((uint64_t *) (rawBase + i)) = rawBase + i - 0x1000;
        
    // Clear bottom page to NULL and set top page as the head of the stack
    *((uint64_t *) rawBase) = 0;
    KMemStackRoot = (void *) (rawBase + length - 0x1000);
}
