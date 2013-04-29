#ifndef KERNEL_ATOMIC_H
#define KERNEL_ATOMIC_H

/*
 * kernel/src/include/atomic.h
 * Atomic Functions
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

// The data type used by spinlocks
typedef struct { volatile int data; } AtomicSpinlock;

// Memory access barrier
static inline void AtomicBarrier()
{
    __sync_synchronize();
}

// Pause instruction (used in spinlocks to relax cpu)
static inline void AtomicPause()
{
    __asm volatile("pause");
}

// Enters the given spinlock
static inline void AtomicLock(AtomicSpinlock * lock)
{
    // Spin until locked
    while(__sync_lock_test_and_set(&lock->data, 1))
    {
        do
        {
            AtomicPause();
        }
        while(lock->data);
    }
}

// Try to enter the given spinlock
static inline bool AtomicTryLock(AtomicSpinlock * lock)
{
    return __sync_lock_test_and_set(&lock->data, 1) == 0;
}

// Leaves the given spinlock
static inline void AtomicUnlock(AtomicSpinlock * lock)
{
    __sync_lock_release(&lock->data);
}

// Initializes a spinlock at runtime (just sets it to 0)
static inline void AtomicLockInit(AtomicSpinlock * lock)
{
    lock->data = 0;
}

// Compare exchange operation
//  Atomicly copies new into *ptr if *ptr == old
//  The original value (whatever it was) is returned
#define AtomicCompareSwap(ptr, old, new) __sync_val_compare_and_swap(ptr, old, new)

// Atomic Addition
//  Atomically adds value to *ptr and returns the new value in *ptr
#define AtomicAdd(ptr, value) __sync_add_and_fetch(ptr, value)

#endif
