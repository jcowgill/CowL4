#
#  Makefile
#  Top-level Makefile for CowL4
#
#  Copyright (C) 2013 James Cowgill
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

# Compilers
CC := clang
AS := amd64-elf-as
LD := amd64-elf-ld

# Global Compiler Options
CF_GLOBAL := -ccc-host-triple amd64-elf -Weverything -g -std=gnu11 -fno-common
AF_GLOBAL := -g
LF_GLOBAL := -n

# Build directory
BUILD_DIR := build

# List of all possible programs
TARGETS := kernel

# Global targets
.PHONY: all clean $(TARGETS)
all: $(TARGETS)

clean:
	rm -r $(BUILD_DIR)

# Inlucde target's rules
include $(addsuffix /Rules.mk, $(TARGETS))
