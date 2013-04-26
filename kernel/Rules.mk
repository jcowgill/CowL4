#
#  kernel/Rules.mk
#  Makefile rules for the kernel
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

# Local flags
CF_LOCAL := -ffreestanding -Ikernel/include -mcmodel=kernel -mno-sse -mno-mmx -msoft-float
AF_LOCAL :=
LF_LOCAL := -T kernel/src/kernel.ld

# Input files
SOURCES  := $(wildcard kernel/src/*.c)      $(wildcard kernel/src/*.s)
SOURCES  += $(wildcard kernel/src/boot/*.c) $(wildcard kernel/src/boot/*.s)

INCLUDES := $(wildcard kernel/include/*.h)

OBJS := $(addprefix $(BUILD_DIR)/, $(addsuffix .o, $(basename $(SOURCES))))

# Build targets
$(BUILD_DIR)/kernel.elf: $(OBJS) kernel/src/kernel.ld
	@mkdir -p $(dir $@)
	$(LD) $(LF_GLOBAL) $(LF_LOCAL) -o $@ $(filter-out %.ld, $^)

$(BUILD_DIR)/kernel/%.o: kernel/%.c $(INCLUDES)
	@mkdir -p $(dir $@)
	$(CC) $(CF_GLOBAL) $(CF_LOCAL) -o $@.s -S $<
	$(AS) -o $@ $@.s

$(BUILD_DIR)/kernel/%.o: kernel/%.s
	@mkdir -p $(dir $@)
	$(AS) $(AF_GLOBAL) $(AF_LOCAL) -o $@ -c $<

# Main target
kernel:	$(BUILD_DIR)/kernel.elf
