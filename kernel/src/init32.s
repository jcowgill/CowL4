/*
 * kernel/src/init32.s
 * 32-bit Kernel Initialization
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

.code32
.intel_syntax noprefix

.global Init32

.bss
    .align 4096

MemKernelTable:
    # Paging tables used by kernel mode
    .skip 4096          # PML4 Table (covers entire address space)
    .skip 4096          # Kernel PDPT (covers 512 GB)
    .skip 4096          # First PDT (covers 1GB)
    .skip 4096          # Second PDT (covers 1GB)

.section .init32, "ax", @progbits
    # Multiboot header
    .long 0x1BADB002    # Magic Number
    .long 2             # Flags (populate memory)
    .long 0xE4524FFC    # Checksum

Init32:
    # Kernel entry point
    xchg bx, bx
    cli
    cld

    # Ensure we're using a multiboot bootloader
    cmp eax, 0x2BADB002
    jne Init32NotMultiboot

    # Attempt to flip ID flag
    mov esp, Init32
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
    jz Init32Not64Bit

    # CPUID Test - High Registers Available
    mov eax, 0x80000000
    cpuid
    test eax, 0x7FFFFFFF
    jz Init32Not64Bit

    # CPUID Test - Long Mode Available
    mov eax, 0x80000001
    cpuid
    test edx, 0x20000000
    jz Init32Not64Bit

    # CPUID Test - IOAPIC Available
    mov eax, 1
    cpuid
    test edx, 0x00000020
    jz Init32Not64Bit

    # Paging Setup - PML4
    mov edx, [Init32MemKernelTable]
    lea eax, [edx + 0x1003]     #= Address of kernel PDPT + 3 (present, writable flags)
    mov [edx], eax              # Set first and last entries of the PML4
    mov [edx + 0xFF8], eax

    # Paging Setup - PDPT Entries
    add eax, 0x1000             # Get address of first PDT (flags are same as before)
    mov [edx + 0x1000], eax     # Set first PDPT entry (each entry covers 1GB)
    mov [edx + 0x1FF0], eax     # Set 2nd to last PDPT entry
    add eax, 0x1000             # Get address of second PDT (flags are same as before)
    mov [edx + 0x1008], eax     # Set second PDPT entry
    mov [edx + 0x1FF8], eax     # Set last PDPT entry

    # Paging Setup - PDT Entries (loop through both tables)
    add edx, 0x2000             # Move to beginning of the first PDT
    mov eax, 0x183              # Initialize with entry for page 0 (present, writable, 2mb, global)
    mov ecx, 1024               # 1024 entries in total (comes to 2GB)

.fillPageEntry:
    mov [edx], eax              # Fill entry
    add edx, 8                  # Advance 1 entry
    add eax, 0x200000           # Advance 2MB
    sub ecx, 1
    jnz .fillPageEntry

    # Enable relevant bits in CR0 and CR4
    mov eax, cr0
    or eax, 0x00040020      # Set NE, AM
    and eax, 0x1FFEFFF1     # Clear MP, EM, TS, WP, NW, CD, PG
    mov cr0, eax

    mov eax, cr4
    or eax, 0x000007A8      # Set DE, PAE, PGE, PCE, OSFXSR, OSXMMEXCPT
    and eax, 0xFFFFFFF8     # Clear VME, PVI, TSD
    mov cr4, eax

    # Load page tables
    mov eax, [Init32MemKernelTable]
    mov cr3, eax

    # Load 64-bit GDT
    lgdt [Init32GdtPtr]

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
    ljmp 0x08:#.jump64Stub

.jump64Stub:
    # This is required since you cannot jump directly to a 64-bit address from 32-bit mode
    .code64
    mov rax, Init64
    jmp rax

    # Initialization Error Handling
    # --------------------

Init32NotMultiboot:
    # Error if loaded by a non-multiboot loader
    mov esi, NotMultibootStr
    jmp Init32Error

Init32Not64Bit:
    # Error if loaded by a non-64bit cpu
    mov esi, Not64BitStr

Init32Error:
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
Init32Gdt:
    # Dummy GDT used for 64-bit transition (replaced in Init64)
    .word 0         # Word to align pointer

Init32GdtPtr:
    .word 16        # 2 GDT entries
    .long Init32Gdt

    .byte 0, 0, 0, 0, 0, 0x9B, 0xA0, 0     #Selector 08h - Ring 0 Code (64-Bits)

    # Stores physical location of kernel paging tables
Init32MemKernelTable:
    .quad MemKernelTable - 0xFFFFFFFF80000000

    # Initiliazation Error strings
NotMultibootStr:
    .asciz "This OS must be loaded by a multiboot boot loader"
Not64BitStr:
    .asciz "This OS requires a 64-bit CPU"
