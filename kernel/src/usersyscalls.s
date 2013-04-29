/*
 * kernel/src/usersyscalls.s
 * User mode system calls
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

.global InfoUserSyscalls, InfoUserSyscallsEnd

    .text
    .align 16

InfoUserSyscalls:
    # The base of the kernel information page (from our perspective)
    .set PrivKipBase, InfoUserSyscalls - 0x1A0

    # Fields in info page
    .quad pscSpaceControl       - PrivKipBase
    .quad pscThreadControl      - PrivKipBase
    .quad pscProcessorControl   - PrivKipBase
    .quad pscMemoryControl      - PrivKipBase
    .quad scIpc                 - PrivKipBase
    .quad scLipc                - PrivKipBase
    .quad scUnmap               - PrivKipBase
    .quad scExchangeRegisters   - PrivKipBase
    .quad scSystemClock         - PrivKipBase
    .quad scThreadSwitch        - PrivKipBase
    .quad scSchedule            - PrivKipBase
    .quad 0

    .align 16
pscSpaceControl:

    .align 16
pscThreadControl:

    .align 16
pscProcessorControl:

    .align 16
pscMemoryControl:

    .align 16
scIpc:

    .align 16
scLipc:

    .align 16
scUnmap:

    .align 16
scExchangeRegisters:

    .align 16
scSystemClock:
    # Read system clock from info page
    mov rax, [rip + (PrivKipBase + 0xA0)]
    ret

    .align 16
scThreadSwitch:

    .align 16
scSchedule:

InfoUserSyscallsEnd:
