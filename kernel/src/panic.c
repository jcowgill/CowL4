/*
 * kernel/src/panic.c
 * Kernel panic handler
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
#include "atomic.h"
#include "ioports.h"
#include "list.h"

// Start of text mode screen buffer
#define SCREEN_START ((int16_t *) 0xFFFFFFFF800B8000)

// Colour for panic messages (0x0C00 = Bright Red on Black Background)
#define SCREEN_COLOUR 0x0C00

// Number of bytes to erase from top of screen (1 line = 160 bytes)
#define SCREEN_ERASE_BYTES (160 * 2)

// Clears the top 2 rows of the screen
static void ClearTopRows()
{
    memset(SCREEN_START, 0, SCREEN_ERASE_BYTES); 
}

// Prints a message in red + returns the new screen pointer
static int16_t * PrintInRed(int16_t * screenPtr, const char * msg)
{
    if (msg)
    {
        while(*msg)
            *screenPtr++ = *msg++ | SCREEN_COLOUR;
    }
        
    return screenPtr;
}

// Moves the screen pointer to the next line
static int16_t * PrintNewLine(int16_t * screenPtr)
{
    return screenPtr + (80 - ((screenPtr - SCREEN_START) % 80));
}

// Halts the processor
static void NO_RETURN Halt()
{
    BREAKPOINT;
    for(;;)
    {
        __asm volatile( "cli\n"
                        "hlt\n");
    }
}

void NO_RETURN Panic(const char * msg)
{
#warning Todo - fix Panic for multiple processors

    // Erase first few lines
    ClearTopRows();

    // Print panic message
    int16_t * screenPtr = SCREEN_START;
    
    screenPtr = PrintInRed(screenPtr, "Kernel Panic: ");
    screenPtr = PrintInRed(screenPtr, msg);
    
    // Halt processor
    Halt();
}

void NO_RETURN PanicAssert(const char * assertion, const char * file, const char * func)
{
    // Erase first few lines
    ClearTopRows();

    // Print panic message
    int16_t * screenPtr = SCREEN_START;
    
    screenPtr = PrintInRed(screenPtr, "Kernel Panic - Assertion Failed: ");
    screenPtr = PrintInRed(screenPtr, assertion);
    screenPtr = PrintNewLine(screenPtr);
    screenPtr = PrintInRed(screenPtr, " at ");
    screenPtr = PrintInRed(screenPtr, file);
    screenPtr = PrintInRed(screenPtr, " ");
    screenPtr = PrintInRed(screenPtr, func);
    
    // Halt processor
    Halt();
}
