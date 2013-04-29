/*
 * kernel/src/infopage.c
 * L4 Kernel Information Page
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
#include "infopage.h"
#include "time.h"

// The global info page
InfoPageType InfoPage ALIGN(4096);

// System calls to be copied over
extern char InfoUserSyscalls, InfoUserSyscallsEnd;

void InfoPageInit(void)
{
    // Fill info page constants
    memcpy(InfoPage.magic, "L4\xE6K", 4);
    InfoPage.version        = CONFIG_L4_VERSION;
    InfoPage.flags          = CONFIG_L4_FLAGS;
    InfoPage.kernDescPtr    = offsetof(InfoPageType, kernId);

    InfoPage.kernId         = CONFIG_L4_ID;
    InfoPage.kernVersion    = CONFIG_VERSION;

    memcpy(InfoPage.kernSupplier, CONFIG_SUPPLIER, sizeof(InfoPage.kernSupplier));
    memcpy(InfoPage.kernExtra, CONFIG_VERSION_EXTRA, sizeof(InfoPage.kernExtra));

    InfoPage.kipSize        = 12;           // = 2^12 = 4096

    InfoPage.clockPrecision = TimeMakePeriod(1000000 / CONFIG_HZ);
    InfoPage.pageInfo       = 0x00201003;   // 2MB and 4KB pages, Write and Execute changable

#warning Todo - fill in utcbInfo and threadInfo
    InfoPage.utcbInfo       = 0;
    InfoPage.threadInfo     = 0;

    // Copy system calls
    memcpy(&InfoPage.pscSpaceControl, &InfoUserSyscalls,
            (uint64_t) (&InfoUserSyscallsEnd - &InfoUserSyscalls));

#warning Todo - memory regions and processors
    // Memory regions

    // Processors
}
