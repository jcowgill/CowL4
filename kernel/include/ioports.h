#ifndef KERNEL_IO_H
#define KERNEL_IO_H

/*
 * kernel/src/include/io.h
 * IO Port Functions
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

static inline void IoOutB(uint16_t port, uint8_t data)
{
    __asm volatile("outb %1, %0"::"Nd"(port), "a"(data));
}

static inline void IoOutW(uint16_t port, uint16_t data)
{
    __asm volatile("outw %1, %0"::"Nd"(port), "a"(data));
}

static inline void IoOutD(uint16_t port, uint32_t data)
{
    __asm volatile("outd %1, %0"::"Nd"(port), "a"(data));
}

static inline uint8_t IoInB(uint16_t port)
{
    unsigned char data;
    __asm volatile("inb %1, %0":"=a"(data):"Nd"(port));
    return data;
}

static inline uint16_t IoInW(uint16_t port)
{
    unsigned char data;
    __asm volatile("inw %1, %0":"=a"(data):"Nd"(port));
    return data;
}

static inline uint32_t IoInD(uint16_t port)
{
    unsigned char data;
    __asm volatile("ind %1, %0":"=a"(data):"Nd"(port));
    return data;
}

#endif
