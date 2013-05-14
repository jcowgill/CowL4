/*
 * kernel/src/boot/start.s
 * Kernel Startup Code (mostly 32-bit)
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

.code32
.intel_syntax noprefix

.global BootStart

.section .boot32, "awx", @progbits
BootSectionStart:
    # Boot stack is stored at the end of the boot section
    .set BootStack, BootSectionStart + 4096

    # Multiboot header
    .long 0x1BADB002    # Magic Number
    .long 2             # Flags (populate memory)
    .long 0xE4524FFC    # Checksum

BootStart:
    # Kernel entry point
    cli
    cld

    # Ensure we're using a multiboot bootloader
    cmp eax, 0x2BADB002
    jne BootNotMultiboot

    # Attempt to flip ID flag
    mov esp, offset BootStack
    pushfd
    pop eax
    mov ecx, eax
    xor eax, 0x200000
    push eax
    popfd

    # CPUID available if the flag could be flipped
    pushfd
    pop eax
    xor eax, ecx
    jz BootNot64Bit

    # CPUID Test - High Registers Available
    mov eax, 0x80000000
    cpuid
    test eax, 0x7FFFFFFF
    jz BootNot64Bit

    # CPUID Test - Long Mode Available
    mov eax, 0x80000001
    cpuid
    test edx, 0x20000000
    jz BootNot64Bit

    # CPUID Test - APIC Available
    mov eax, 1
    cpuid
    test edx, 0x00000200
    jz BootNot64Bit

    # Paging Setup - PML4
    mov edx, [BootMemKernelTable]
    mov cr3, edx                # Load PML4 address into cr3

    lea eax, [edx + 0x1003]     # Address of kernel PDPT + 3 (present, writable flags)
    mov [edx], eax              # Set first and last entries of the PML4
    mov [edx + 0xFF8], eax

    # Paging Setup - PDPT Entries
    #  1 entry in upper half (kernel code)
    add eax, 0x1000             # Address of first PDT + 3 (present, writable)
    mov [edx + 0x1FF0], eax

    #  4 entries in lower half (kernel physical memory access)
    mov [edx + 0x1000], eax
    add eax, 0x1000
    mov [edx + 0x1008], eax
    add eax, 0x1000
    mov [edx + 0x1010], eax
    add eax, 0x1000
    mov [edx + 0x1018], eax

    # Paging Setup - PDT Entries (loop through both tables)
    add edx, 0x2000             # Move to beginning of the first PDT
    mov eax, 0x183              # Initialize with entry for page 0 (present, writable, 2mb, global)
    mov ecx, 0x1000             # 4096 entries in total (comes to 4GB)

BootFillPageEntry:
    mov [edx], eax              # Fill entry
    add edx, 8                  # Advance 1 entry
    add eax, 0x200000           # Advance 2MB
    sub ecx, 1
    jnz BootFillPageEntry

    # Enable relevant bits in CR0 and CR4
    mov eax, cr0
    or eax, 0x00040020      # Set NE, AM
    and eax, 0x1FFEFFF1     # Clear MP, EM, TS, WP, NW, CD, PG
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

    # Enable Paging
    mov eax, cr0
    or eax, 0x80000000
    mov cr0, eax

    # Jump to 64-bit segment
    ljmp 0x08, Boot64Start

Boot64Start:
    # This is required since you cannot jump directly to a 64-bit address from 32-bit mode
    .code64

    # Fixup multiboot header address
    or ebx, 0x80000000
    movsx rdi, ebx

    # Load boot stack
    mov rsp, offset BootStack + 0xFFFFFFFF80000000

    # Call main 64 bit code
    mov rax, offset BootMain
    call rax

    # Initialization Error Handling
    # --------------------

    .code32

BootNotMultiboot:
    # Error if loaded by a non-multiboot loader
    mov esi, offset NotMultibootStr
    jmp BootError

BootNot64Bit:
    # Error if loaded by a non-64bit cpu
    mov esi, offset Not64BitStr

BootError:
    # Fatal initialization error
    #  Message provided in esi

    mov eax, 0x00000C00     # 0C is used as the colour code
    mov edi, 0xB8000        # Console output buffer

.loopStart:
    mov al, [esi]           # Read character
    test al, al             # Test for nulls
    jz .finished

    mov [edi], ax           # Write character + colour code
    inc esi
    add edi, 2
    jmp .loopStart

.finished:
    # Halt
    cli
    hlt
    jmp .finished

    # Initialization Data
    # --------------------

    .align 8
BootGdt:
    # Dummy GDT used for 64-bit transition (replaced in Init64)
    .word 0         # Word to align pointer

BootGdtPtr:
    .word 16        # 2 GDT entries
    .long BootGdt

    .byte 0, 0, 0, 0, 0, 0x9B, 0xA0, 0     #Selector 08h - Ring 0 Code (64-Bits)

    # Stores physical location of kernel paging tables
BootMemKernelTable:
    .quad MemKernelTables - 0xFFFFFFFF80000000

    # Initiliazation Error strings
NotMultibootStr:
    .asciz "This OS must be loaded by a multiboot boot loader"
Not64BitStr:
    .asciz "This OS requires a 64-bit CPU"
