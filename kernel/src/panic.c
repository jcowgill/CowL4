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
 * but WITHOUT ANY WARRANTY# without even the implied warranty of
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

// Number of characters to erase from top of screen (80 chars is one line)
#define SCREEN_ERASE_AMOUNT (80 * 2)

void NO_RETURN Panic(const char * msg)
{
    // TODO Fix this for multiple processors

    // Erase first few lines
    memset(SCREEN_START, 0, SCREEN_ERASE_AMOUNT * 2);

    // Print message prefix
    int16_t * screenPtr = SCREEN_START;
    char * msgPrefix = "Kernel Panic: ";

    while(*msgPrefix)
        *screenPtr++ = *msgPrefix++ | SCREEN_COLOUR;

    // Print main message
    if (msg != NULL)
    {
        while(*msg)
            *screenPtr++ = *msg++ | SCREEN_COLOUR;
    }

    // Halt processor
    BREAKPOINT;
    for(;;)
    {
        __asm volatile( "cli\n"
                        "hlt\n");
    }
}
