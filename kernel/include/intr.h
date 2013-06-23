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

// Interrupt context (all registers + information saved on an interrupt)
typedef struct IntrContext
{
    // General Purpose Registers
    uint64_t rax;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;

    // Interrupt Information
    uint64_t intrNumber;
    uint64_t intrError;

    // Information pushed by the CPU
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;

} IntrContext;

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

// Interrupt number constants
#define INTR_CPU_DE         0       // Division by 0
#define INTR_CPU_DB         1       // Debug Exception
#define INTR_CPU_BP         3       // Breakpoint
#define INTR_CPU_UD         6       // Invalid Opcode
#define INTR_CPU_NM         7       // FPU Not Available
#define INTR_CPU_DF         8       // Double Fault
#define INTR_CPU_TS         10      // Invalid TSS
#define INTR_CPU_NP         11      // Segment Not Present
#define INTR_CPU_SS         12      // Stack Fault
#define INTR_CPU_GP         13      // General Protection Fault
#define INTR_CPU_PF         14      // Page Fault
#define INTR_CPU_MF         16      // FPU Exception
#define INTR_CPU_AC         17      // Alignment Check
#define INTR_CPU_MC         18      // Machine Check
#define INTR_CPU_XM         19      // SIMD Exception

#define INTR_APIC_SPURIOUS  32      // APIC Spurious Interrupt
#define INTR_APIC_TIMER     33      // APIC Local Timer

#define INTR_IRQ            48      // Hardware Interrupt

// Pointer to the IDT
extern const IntrIdtPtrType IntrIdtPtr;

// Creates the interrupt descriptor table
void IntrInitIdt(void);

// Initializes the PIC
void IntrInitPic(void);

// Interrupt handler entry point
void IntrHandler(IntrContext context);

#endif
