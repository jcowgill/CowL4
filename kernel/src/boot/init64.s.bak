/*
 * kernel/src/init64.s
 * 64-bit Kernel Initialization
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

.global Init64

.text
Init64:
    # Code reaches this point once long mode and paging is enabled

    # Clear unused segments
    xor rcx, rcx
    mov ds, cx
    mov es, cx
    mov fs, cx
    mov gs, cx
    mov ss, cx

    # Init FPU and MXCSR
    fninit
    ldmxcsr [InitMXCSR]

    # Load gdt, tr and idt
    # Syscalls depend on GDT being
    #  00 = Nothing
    #  08 = Kernel Code
    #  10 = Kernel Data
    #  1B = User DATA !!!! - THIS IS THE OTHER WAY AROUND
    #  23 = User CODE

    #  Note that a different tss is needed for each processor (different kernel stacks)
    lidt [InitIdtPtr]

    # Setup global MSRs
    xor eax, eax
    mov edx, 0x00130008     # SYSCALL and SYSRET segments
    mov ecx, 0xC0000081     # MSR_STAR
    wrmsr

    mov rax, EntrySyscall   # Syscall entry point
    mov rdx, rax
    shr rdx, 32
    mov ecx, 0xC0000082     # MSR_LSTAR
    wrmsr

    xor rdx, rdx
    mov eax, 0x00000700     # Clear DF, TF, IF on SYSCALL
    mov ecx, 0xC0000082     # MSR_FMASK
    wrmsr

    # STAR = C0000081 (SYSCALL segments numbers)
    # LSTAR = C0000082 (SYSCALL target address - 64bit VMA)
    # FMASK = C0000084 (SYSCALL flag mask)
    # FS_BASE = C0000100 (FS segment base VMA)
    # GS_BASE = C0000101 (GS segment base VMA)
    # KERNEL_GS_BASE = C0000102 (GS kernel segment base VMA - used by SWAPGS)

    # Setup interrupts (should lidt be here??)

    # Enable machine check exception

    # Parse multiboot header

    # Init other CPUS, IOAPIC and System Clock


    xchg bx, bx
    hlt

InitMXCSR:
    .long 0x1F80    # Initial value for the MXCSR register

    # Ensure alignment is correct
    .align 16
    .skip 6

InitIdtPtr:
    .word 4096      # 256 x 16 byte entries
    .quad IntrIdt
