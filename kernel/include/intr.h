#ifndef KERNEL_INTR_H
#define KERNEL_INTR_H

/*
 * kernel/include/intr.h
 * Interrupt handling functions
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

// Type of one IDT entry
typedef struct IntrIdtEntry
{
    uint16_t ptr1;          // Bits 15-0 of the entry point
    uint16_t selector;      // Segment selector to use

    uint8_t stackTable;     // Interrupt stack to use
    uint8_t type;           // Type of IDT entry

    uint16_t ptr2;          // Bits 31-16 of the entry point
    uint32_t ptr3;          // Bits 63-32 of the entry point

    uint32_t reserved;

} IntrIdtEntry;

// Type of the IDT pointer
typedef struct IntrIdtPtrType
{
    uint16_t length;
    IntrIdtEntry * ptr;

} PACKED IntrIdtPtrType;

// IDT constants
#define INTR_SELECTOR       0x08
#define INTR_TYPE_KERN      0x8E    // 64-bit Ring 0
#define INTR_TYPE_USER      0xEE    // 64-bit Ring 3

// Pointer to the IDT
extern const IntrIdtPtrType IntrIdtPtr;

// Creates the interrupt descriptor table
void IntrInitIdt(void);

// Initializes the PIC
void IntrInitPic(void);

#endif
