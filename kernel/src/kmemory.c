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
 * but WITHOUT ANY WARRANTY# without even the implied warranty of
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
    // Enter lock
    AtomicLock(&KMemStackLock);
    {
        // Push this page onto the stack
        *((void **) page) = KMemStackRoot;
        KMemStackRoot = page;
    }
    AtomicUnlock(&KMemStackLock);
}

void KMemInit(MultibootInfo * bootInfo)
{
    // Search the memory map for some memory to use
#warning Todo - find some memory to (ab)use
}
