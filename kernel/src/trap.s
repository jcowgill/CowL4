/*
 * kernel/src/trap.s
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
 * but WITHOUT ANY WARRANTY# without even the implied warranty of
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

.bss
IntrIdt:
    # Interrupt descriptor table
    .align 4096
    .skip 4096

.text
IntrSetup:
    # Initializes the interrupt descriptor table

    # Reset loop counter
    xor rcx, rcx

.idtLoopStart:
    # Construct lower half in rax
    mov rdx, IntrEntryStart
    lea rdx, [IntrEntryStart + rcx]

    mov rax, rdx
    mov ax, 0x8E00      # Entry Type (64-bit Ring 0 Interrupt)
    shl rax, 16
    mov al, 0x08        # Destination Segment
    shl rax, 16
    mov ax, dx

    # Save lower half
    mov [IntrIdt + 2 * rcx], rax

    # Save higher half
    mov dword ptr [IntrIdt + 2 * rcx + 8], 0xFFFFFFFF

    # Loop until counter is too high
    add rcx, 8
    cmp rcx, 0x1000
    jne .idtLoopStart

    # Allow Ring 3 code to issue breakpoints
    mov byte ptr [IntrIdt + (16 * 3 + 5)], 0xEE
                        # Entry Type (64-bit Ring 3 Interrupt)

    ret

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

    IsrCode        0, IntrEntryCommon
    IsrCode        1, IntrEntryPanicNoError
    IsrCode        2, IntrEntryCommon
    IsrCode        3, IntrEntryCommon
    IsrCodeIgnore #4
    IsrCodeIgnore #5
    IsrCode        6, IntrEntryCommon
    IsrCode        7, IntrEntryCommon
    IsrCode        8, IntrEntryPanic
    IsrCodeIgnore #9
    IsrCode       10, IntrEntryPanic
    IsrCode       11, IntrEntryErrorCode
    IsrCode       12, IntrEntryErrorCode
    IsrCode       13, IntrEntryErrorCode
    IsrCode       14, IntrEntryErrorCode
    IsrCode       15, IntrEntryCommon
    IsrCode       16, IntrEntryCommon
    IsrCode       17, IntrEntryErrorCode
    IsrCode       18, IntrEntryPanicNoError
    IsrCode       19, IntrEntryCommon

    # Ignore unused exceptions
.rept 32 - 20
    IsrCodeIgnore
.endr

    # Default entries for IRQ interrupts
.set i, 18
.rept 256 - 32
    IsrCode i, IntrEntryCommon
    .set i, i + 1
.endr

IntrEntryErrorCode:
    # Common code for interrupts producing an error
    xchg bx, bx
    hlt

    # Complete interrupt
    pop rax
    add esp, 8
    iretq

IntrEntryCommon:
    # Common code for interrupts not producing an error
    xchg bx, bx
    hlt


    # Complete interrupt
    pop rax
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
