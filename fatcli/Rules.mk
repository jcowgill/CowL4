#
#  fatcli/Rules.mk
#  Makefile rules for fatcli
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
CF_$(dir)   := -I$(dir)/include

# Find sources and includes
SRC_$(dir)  := $(wildcard $(dir)/src/*.c)
INC_$(dir)  := $(wildcard $(dir)/include/*.h)

# Generate objects list
OBJ_$(dir)  := $(call GEN_OBJS, $(SRC_$(dir)))

# C Sources depend on all includes (the simple way)
$(SRC_$(dir)):  $(INC_$(dir))

###############

# Set build flags
$(OBJ_$(dir)):  CF_LOCAL := $(CF_$(dir))

# Linking rules
$(BUILD_DIR)/$(dir).elf: $(OBJ_$(dir))
	$(LINK_NATIVE)

$(dir): $(BUILD_DIR)/$(dir).elf
