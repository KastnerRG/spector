# ----------------------------------------------------------------------
# Copyright (c) 2016, The Regents of the University of California All
# rights reserved.
# 
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are
# met:
# 
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
# 
#     * Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials provided
#       with the distribution.
# 
#     * Neither the name of The Regents of the University of California
#       nor the names of its contributors may be used to endorse or
#       promote products derived from this software without specific
#       prior written permission.
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL REGENTS OF THE
# UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
# OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
# TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
# USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
# DAMAGE.
# ----------------------------------------------------------------------



# You must configure ALTERAOCLSDKROOT to point the root directory of the Altera SDK for OpenCL
# software installation.
# See http://www.altera.com/literature/hb/opencl-sdk/aocl_getting_started.pdf 
# for more information on installing and configuring the Altera SDK for OpenCL.

ifeq ($(VERBOSE),1)
ECHO := 
else
ECHO := @
endif

# Where is the Altera SDK for OpenCL software?
#ifeq ($(wildcard $(ALTERAOCLSDKROOT)),)
#$(error Set ALTERAOCLSDKROOT to the root directory of the Altera SDK for OpenCL software installation)
#endif
#ifeq ($(wildcard $(ALTERAOCLSDKROOT)/host/include/CL/opencl.h),)
#$(error Set ALTERAOCLSDKROOT to the root directory of the Altera SDK for OpenCL software installation.)
#endif

# Compilation flags
ifeq ($(DEBUG),1)
CXXFLAGS += -g
else
CXXFLAGS += -O2
endif

CXXFLAGS += -Wno-unused-result


# Compiler
CXX := g++

# Target
TARGET := tdfir
TARGET_DIR := bin

# Directories
INC_DIRS := host/inc ../common/inc
LIB_DIRS := 

# Files
INCS := $(wildcard host/inc/*.h)
SRCS := $(wildcard host/src/*.cpp ../common/src/AOCLUtils/*.cpp)
LIBS := 


# Makefile targets
.PHONY: all clean cpu gpu fpga

fpga: CL_COMPILE_CONFIG = $(shell aocl compile-config)
fpga: CL_LINK_CONFIG    = $(shell aocl link-config | sed 's/lterasic/laltera/g')
fpga: $(TARGET_DIR)/$(TARGET)

gpu: CL_COMPILE_CONFIG = -I/usr/local/cuda/include
gpu: CL_LINK_CONFIG    = -L/usr/local/cuda/lib64 -lOpenCL
gpu: $(TARGET_DIR)/$(TARGET)

cpu: gpu

all: fpga

# Host executable target.
$(TARGET_DIR)/$(TARGET) : Makefile $(SRCS) $(INCS)
	@[ -d $(TARGET_DIR) ] || mkdir $(TARGET_DIR)
	$(ECHO)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -fPIC $(foreach D,$(INC_DIRS),-I$D) \
			$(CL_COMPILE_CONFIG) $(SRCS) $(CL_LINK_CONFIG)  \
			$(foreach D,$(LIB_DIRS),-L$D) \
			$(foreach L,$(LIBS),-l$L) \
			-o $(TARGET_DIR)/$(TARGET)

# Standard make targets
clean :
	$(ECHO)rm -f $(TARGET_DIR)/$(TARGET)


# Copyright (C) 2013-2014 Altera Corporation, San Jose, California, USA. All rights reserved.
# Permission is hereby granted, free of charge, to any person obtaining a copy of this
# software and associated documentation files (the "Software"), to deal in the Software
# without restriction, including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
# whom the Software is furnished to do so, subject to the following conditions:
# The above copyright notice and this permission notice shall be included in all copies or
# substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
# OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
# WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
# 
# This agreement shall be governed in all respects by the laws of the State of California and
# by the laws of the United States of America.
# This is a GNU Makefile.

