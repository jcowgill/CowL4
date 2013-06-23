/*
 * kernel/src/intr_asm.s
 * Entry points into the kernel
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

.code64
.intel_syntax noprefix

.global EntrySyscall
# All ISRs are also global

.text

.macro IsrNormal, num:req
    # Normal ISR (without error code)
.global IntrIsr\num
IntrIsr\num:
    push 0
    push \num
    jmp IntrEntry
.endm

.macro IsrErrorCode, num:req
    # ISR with an error code
.global IntrIsr\num
IntrIsr\num:
    push \num
    jmp IntrEntry
.endm

    # CPU Exceptions
    IsrNormal       0       # DE  - Division By Zero
    IsrNormal       1       # DB  - Debug Exception
    #IsrNormal      2       # NMI - Non Maskable Interrupt
    IsrNormal       3       # BP  - Breakpoint
    #IsrNormal      4       # OF  - Overflow (impossible in 64-bit mode)
    #IsrNormal      5       # BR  - Bound Range Exceeded (impossible in 64-bit mode)
    IsrNormal       6       # UD  - Invalid Opcode
    IsrNormal       7       # NM  - FPU Not Available
    IsrNormal       8       # DF  - Double Fault
    #IsrNormal      9       # --  - Coprocessor Overrun (impossible in 64-bit mode)
    IsrNormal       10      # TS  - Invalid TSS
    IsrErrorCode    11      # NP  - Segment Not Present
    IsrErrorCode    12      # SS  - Stack Fault
    IsrErrorCode    13      # GP  - General Protection Fault
    IsrErrorCode    14      # PF  - Page Fault
    #IsrNormal      15      # --  - Reserved
    IsrNormal       16      # MP  - FPU Exception
    IsrErrorCode    17      # AC  - Alignment Check
    IsrNormal       18      # MC  - Machine Check
    IsrNormal       19      # XM  - SIMD Exception
    #IsrNormal      20      # VE  - Virtualization Exception

    # APIC Interrupts
    IsrNormal       32      # Spurious Interrupt
    IsrNormal       33      # Timer Interrupt

    # Hardware interrupts (all use one isr)
    IsrNormal       48      # Hardware interrupts

IntrEntry:
    # Interrupt entry point
    #  Error code and interrupt number already pushed on the stack

    # Push anything which isn't saved across C function calls
    push r11
    push r10
    push r9
    push r8
    push rdi
    push rsi
    push rdx
    push rcx
    push rax

    # Call interrupt handling function
    call IntrHandler

    # Pop registers
    pop rax
    pop rcx
    pop rdx
    pop rsi
    pop rdi
    pop r8
    pop r9
    pop r10
    pop r11

    # Pop interrupt and error numbers, and complete interrupt
    add rsp, 16
    iretq

IntrEntryPanicNoError:
IntrEntryPanic:
    # Interrupt which causes a panic (with and without error code)
    #  TODO Machine check + double fault etc MUST use an interrupt stack to
    #   avoid race condition when executing SYSRET (otherwise they will
    #   be executed on the user stack)
    xchg bx, bx
    hlt

EntrySyscall:
    # Syscall entry point
    #  rcx = user mode rip
    #  r11 = user mode rflags
    #  rsp = USER MODE STACK (still)

    xchg bx, bx
    sysret
