/*
 * kernel/src/boot/cpu_init_asm.s
 * CPU Initialization for APs
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

.intel_syntax noprefix
.global CpuLowerInit, CpuLowerInitEnd, CpuLateInitAsm

.set CPU_LOW_INIT_LOC, 0x8000

###############################################
#  16-bit code (at 0x8000)
###############################################

.section .boot32, "awx", @progbits
    .align 16
    .code16
CpuLowerInit:
    # 16-bit AP entry point
    # Jump into 32-bit mode
    cli
    cld

    xor ax, ax
    mov ds, ax

    lgdt [CPU_LOW_INIT_LOC + (Gdt32Ptr - CpuLowerInit)]

    mov eax, cr0
    or al, 1
    mov cr0, eax

    ljmp 0x08, CPU_LOW_INIT_LOC + (Trampoline32 - CpuLowerInit)

Trampoline32:
    # Trampoline to get into 32-bit mode
    .code32
    ljmp 0x08, CpuInit32

    .align 8
Gdt32:
    # 32-bit temporary GDT
    .word 0

Gdt32Ptr:
    .word 24 - 1        # 3 GDT entries
    .long CPU_LOW_INIT_LOC + (Gdt32 - CpuLowerInit)

	.byte 0xFF, 0xFF, 0, 0, 0, 0x9A, 0xCF, 0	# Selector 08h - Ring 0 Code
	.byte 0xFF, 0xFF, 0, 0, 0, 0x92, 0xCF, 0	# Selector 10h - Ring 0 Data

    .align 16
CpuLowerInitEnd:

###############################################
#  32-bit code (at .boot32 = around 0x100000)
###############################################

CpuInit32:
    # 32-bit entry
    mov ax, 0x10
    mov ds, ax

    # Enable relevant bits in CR0 and CR4
    mov eax, cr0
    or eax, 0x00040022      # Set NE, AM, MP
    and eax, 0x1FFEFFF3     # Clear EM, TS, WP, NW, CD, PG
    mov cr0, eax

    mov eax, cr4
    or eax, 0x000007A8      # Set DE, PAE, PGE, PCE, OSFXSR, OSXMMEXCPT
    and eax, 0xFFFFFFF8     # Clear VME, PVI, TSD
    mov cr4, eax

    # Load 64-bit GDT
    lgdt [BootGdtPtr]

    # Enable long mode (and other stuff) in EFER MSR
    mov ecx, 0xC0000080
    rdmsr
    or eax, 0x00000901      # Set SYSCALL, IA-32e, NX
    wrmsr

    # Load PML4 address
    mov edx, [BootMemKernelTable]
    mov cr3, edx

    # Enable Paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    # Jump to 64-bit segment
    ljmp 0x08, Boot64Trampoline

Boot64Trampoline:
    # Trampoline to get into 64-bit mode
    .code64
    jmp Boot64Start

###############################################
#  64-bit code (at .text = around 0xFFF...)
###############################################

    .text
    .align 16
Boot64Start:
    # This is required since you cannot jump directly to a 64-bit address from 32-bit mode
    .code64

    # Get CPU strcuture
    mov rax, [CpuLocalApic]
    mov eax, [rax + 0x20]               # Read APIC register 20
    shr eax, 24
    mov eax, [CpuApicToCpuId + rax*4]   # Get CPU ID from APIC ID
    mov rdi, [CpuList + rax*8]          # Get CPU structure in rdi

    # Load boot stack for this CPU (the end of the CPU structure)
    lea rsp, [rdi + 0x1000]

    # Call C entry point (cpu structure is parameter 1)
    call CpuApEntry

CpuLateInitAsm:
    # Finishes the initialization of the current CPU
    #  void CpuLateInitAsm(void * gdtPtr);

    # Reset FPU
    fninit

    # Load GDT, TSS and IDT
    lgdt [rdi]
    mov ax, 0x28
    ltr ax
    lidt [IntrIdtPtr]

    # Enable machine check exception (now that we have interrupts)
    mov rax, cr4
    or rax, 0x00000040      # Set MCE
    mov cr4, rax

    # Clear (unused) data selectors
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    # Setup syscall MSRs
    xor eax, eax
    mov edx, 0x00130008     # SYSCALL and SYSRET segments
    mov ecx, 0xC0000081     # MSR_STAR
    wrmsr

    mov rax, offset EntrySyscall
    mov rdx, rax
    shr rdx, 32
    mov ecx, 0xC0000082     # MSR_LSTAR
    wrmsr

    xor rdx, rdx
    mov eax, 0x00000700     # Clear DF, TF, IF on SYSCALL
    mov ecx, 0xC0000082     # MSR_FMASK
    wrmsr

    ret
