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

# Use clang unless overridden
CC          := clang

# Global Compiler Options
CF_GLOBAL   := -Wall -Wextra -g -std=gnu11 -fno-common
AF_GLOBAL   := -g
LF_GLOBAL   := -n

# Build directory
BUILD_DIR   := build

# List of all projects to build
TARGETS     := kernel fatcli

# Build command lines
#  COMPILE      = C Compiler
#  ASSEMBLE     = Gnu Assembler
#  LINK         = Gnu Linker
#  LINK_NATIVE  = LD via GCC (for executables running on build host)
MKDIR       = @mkdir -p $(dir $@)
COMPILE     = $(MKDIR); $(CC) $(CF_GLOBAL) $(CF_LOCAL) -o $@ -c $<
ASSEMBLE    = $(MKDIR); $(AS) $(AF_GLOBAL) $(AF_LOCAL) -o $@ -c $<
LINK        = $(MKDIR); $(LD) $(LF_GLOBAL) $(LF_LOCAL) -o $@ $(filter-out %.ld, $^)
LINK_NATIVE = $(MKDIR); $(CC) $(LF_LOCAL) -o $@ $^

# Global functions
#  GEN_OBJS = Generates a list of .o files from a list of sources
GEN_OBJS    = $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(1))))

# Add all target first
.PHONY: all $(TARGETS)
all: $(TARGETS)

# Include all project rule files
$(foreach dir, $(TARGETS), $(eval include $(dir)/Rules.mk))

# General build patterns
.SUFFIXES:

$(BUILD_DIR)/%.o: %.c
	$(COMPILE)

$(BUILD_DIR)/%.o: %.s
	$(ASSEMBLE)

# Additional global targets
.PHONY: clean
clean:
	rm -r $(BUILD_DIR)

# This currently installs kernel.elf into the grub2.img file (1048576 = 512 * 2048)
.PHONY: install
install: kernel fatcli
	$(BUILD_DIR)/fatcli.elf grub2.img write kernel.elf build/kernel.elf
