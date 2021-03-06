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

# Compiler options
CF_$(dir)   := -ffreestanding -I$(dir)/include -mcmodel=kernel -mno-sse -mno-mmx -msoft-float
LF_$(dir)   := -T $(dir)/src/kernel.ld

# Find sources and includes
SRC_$(dir)  := $(wildcard $(dir)/src/*.c) $(wildcard $(dir)/src/boot/*.c)
INC_$(dir)  := $(wildcard $(dir)/include/*.h)
SRCA_$(dir) := $(wildcard $(dir)/src/*.s) $(wildcard $(dir)/src/boot/*.s)

# Generate objects list
OBJ_$(dir)  := $(call GEN_OBJS, $(SRC_$(dir)))
OBJA_$(dir) := $(call GEN_OBJS, $(SRCA_$(dir)))

# List if linking inputs
LINK_IN_$(dir)  := $(OBJ_$(dir)) $(OBJA_$(dir)) $(dir)/src/kernel.ld

#############

# C Sources depend on all includes (the simple way)
$(SRC_$(dir)):  $(INC_$(dir))

# Set build flags
$(OBJ_$(dir)):  CF_LOCAL := $(CF_$(dir))

# Linking rules
$(BUILD_DIR)/$(dir).elf: LF_LOCAL := $(LF_$(dir))
$(BUILD_DIR)/$(dir).elf: $(LINK_IN_$(dir))
	$(LINK)

$(dir): $(BUILD_DIR)/$(dir).elf
