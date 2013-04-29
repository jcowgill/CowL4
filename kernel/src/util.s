/*
 * kernel/src/util.s
 * Utility functions
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

.global memcpy, memset

.text
    .align 16
memcpy:
    # void * memcpy(void * restrict dest, const void * restrict src, uint64_t len)
    #  rdi = dest
    #  rsi = src
    #  rdx = len
    #  return dest in rax

    # Save dest for later
    mov rax, rdi

    # Use movsq as much as possible
    mov rcx, rdx
    shr rcx, 3
    rep movsq

    # Use movsb for the rest
    mov rcx, rdx
    and rcx, 3
    rep movsb

    ret

    .align 16
memset:
    # void * memset(void * dest, unit8_t value, uint64_t len)
    #  rdi = dest
    #  rsi = value
    #  rdx = len
    #  return dest in rax

    # Store dest for later and move value to rax
    mov r8, rdi
    mov eax, esi

    # Memset for 0 (very common) requires no extra setup so we can just skip to stosq
    test rsi, rsi
    jz memset.large

    # Skip to stosb for small sets
    cmp rdx, 16
    jb memset.small

    # Copy value to 8 places within rax
    #  (convert 0x0000000000000042 to 0x4242424242424242)
    mov ah, al

    mov si, ax
    shl rax, 16
    mov ax, si

    mov esi, eax
    shl rax, 32
    or rax, rsi     # mov eax, esi would zero extend into top half

memset.large:
    # Use stosq as much as possible
    mov rcx, rdx
    shr rcx, 3
    rep stosq

memset.small:
    # Use stosb for small parts
    mov rcx, rdx
    and rcx, 3
    rep movsb

    # Restore dest into rax
    mov rax, r8
    ret
