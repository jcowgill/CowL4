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
.global IntrIdt
.global IntrSetup

.global IntrEntryStart

.text
IntrEntryStart:
    # Code for each interrupt entry point
    #  All isr codes are a fixed 8 bytes

    # Exception Handling
    #
    # Special kernel handling
    # --------
    # 14 PF  Page Fault
    #  2 NMI (for cross processor invlpg?)
    #  7 NM  Device not available (do FPU state switch)
    #
    # Causes panic (always)
    # --------
    #  1 DB Debug Exception
    #  8 DF Double Fault
    # 10 TS Invalid TSS
    # 18 MC Machine Check
    #
    # Ignored (always)
    # --------
    #  4 OF Overflow (should be impossible to generate)
    #  5 BR Bound Range (should be impossible to generate)
    #  9    Coprocessor Segment Overrun (Only used on 386 processors)
    # 20-31 No Exception Produced
    #
    # ALways handled by the thread's exception handler
    #  - Except when produced from kernel code (causes panic then)
    # --------
    #  0 DE Division By 0
    #  3 BP Breakpoint
    #  6 UD Invalid Opcode
    # 11 NP Segment Not Present
    # 12 SS Stack Fault
    # 13 GP General Protection
    # 16 MF x86 FPU Exception
    # 17 AC Alignment Check
    # 19 XM SIMD Exception
    #

.macro IsrCode, num:req, jump:req
    # Save isr number and jump to common entry point
    push rax        # 1 byte
    mov al, \num    # 2 bytes
    jmp.d32 \jump   # 5 bytes
.endm

.macro IsrCodeIgnore, num
    # Ignore interrupt (no error code)
    iretq           # 2 bytes
    .skip 6         # 6 bytes
.endm

    IsrCode        0, IntrEntryNormal
    IsrCode        1, IntrEntryPanicNoError
    IsrCode        2, IntrEntryNormal
    IsrCode        3, IntrEntryNormal
    IsrCodeIgnore #4
    IsrCodeIgnore #5
    IsrCode        6, IntrEntryNormal
    IsrCode        7, IntrEntryNormal
    IsrCode        8, IntrEntryPanic
    IsrCodeIgnore #9
    IsrCode       10, IntrEntryPanic
    IsrCode       11, IntrEntryErrorCode
    IsrCode       12, IntrEntryErrorCode
    IsrCode       13, IntrEntryErrorCode
    IsrCode       14, IntrEntryErrorCode
    IsrCode       15, IntrEntryNormal
    IsrCode       16, IntrEntryNormal
    IsrCode       17, IntrEntryErrorCode
    IsrCode       18, IntrEntryPanicNoError
    IsrCode       19, IntrEntryNormal

    # Ignore unused exceptions
.rept 32 - 20
    IsrCodeIgnore
.endr

    # Default entries for IRQ interrupts
.set i, 32
.rept 256 - 32
    IsrCode i, IntrEntryNormal
    .set i, i + 1
.endr

IntrEntryNormal:
    # Change stack from (<blank> rax)
    #                to (    rax rsi)
    xchg rsi, [rsp]
    push rsi

    # Jump to common entry point
    jmp IntrEntryNormal

IntrEntryErrorCode:
    # Change stack from (rax <error>)
    #                to (rax rsi    )
    xchg rsi, [rsp + 8]

IntrEntryCommon:
    # Common interrupt entry code
    #  Push the rest of the state (rax and rsi already saved)
    push rcx
    push rdx
    push rdi
    push r8
    push r9
    push r10
    push r11
    swapgs

    # Move error number to first argument
    movzx rdi, al

    # Call interrupt C routine
    #  arg1 = interrupt number
    #  arg2 = error code
    xchg bx, bx

    # Restore state and return
    swapgs
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rdx
    pop rcx

    pop rax
    pop rsi
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
