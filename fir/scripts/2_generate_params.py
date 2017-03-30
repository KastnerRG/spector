#!/usr/bin/python

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
# Filename: 2_generate_params.py
# Version: 1.0
# Description: Python script to generate unique designs for the FIR filter benchmark.
# Author: Quentin Gautier


import itertools
import sys
import math

sys.path.append("../../common/scripts")
from generateDesigns import createFolders



templateFilepath = "../src/parameters.h.template" # Knobs template file
kernelFilename   = "../src/device/tdfir.cl"       # Kernel file to copy 
dirToCopy        = "../benchmarks/basefolder"     # Directory containing the source code

outRootPath        = "../benchmarks"         # Name of output directory where all folders are generated
outBasename        = "tdfir"                 # Base name of generated folders
outKnobFilename    = "host/inc/parameters.h" # Name of generated knob file
outKernelSubfolder = "device/"               # Name of subfolder where to place the generated kernel file

logFilename = "params.log" # Log file to copy useful information


# ***************************************************************************
# Knobs
# ***********

FULL_UNROLL = -1

FILTER_LENGTH = 32
NUM_FILTERS   = 64
FILTER_LENGTH_LOG = int(math.log(FILTER_LENGTH, 2))
NUM_FILTERS_LOG   = int(math.log(NUM_FILTERS, 2))


KNOB_COEF_SHIFT = [1, 8, FILTER_LENGTH] #[1, 8, 32]
KNOB_NUM_PARALLEL = range(1, 9) #[1, 2, 3, 4, 5, 6, 7, 8]

KNOB_UNROLL_FILTER_1     = [FULL_UNROLL] 
KNOB_UNROLL_FILTER_2     = [FULL_UNROLL] 
KNOB_UNROLL_FILTER_3     = [2**x for x in range(FILTER_LENGTH_LOG+1)] #[1, 2, 4, 8, 16, 32]
KNOB_UNROLL_COEF_SHIFT_1 = [FULL_UNROLL]
KNOB_UNROLL_COEF_SHIFT_2 = [FULL_UNROLL]
KNOB_UNROLL_TOTAL        = [1, 2]
KNOB_UNROLL_PARALLEL_1   = [FULL_UNROLL] 
KNOB_UNROLL_PARALLEL_2   = [FULL_UNROLL] 
KNOB_UNROLL_PARALLEL_3   = [FULL_UNROLL]

KNOB_NUM_WORK_ITEMS  = [1, 2, 4, 16, 32, 64]
KNOB_NUM_WORK_GROUPS = [1, 2, 4, 16, 32, 64]

KNOB_SIMD          = [1, 2]
KNOB_COMPUTE_UNITS = [1, 2]




allCombinations = list(itertools.product(
    KNOB_COEF_SHIFT,          # 1
    KNOB_NUM_PARALLEL,        # 2
    KNOB_UNROLL_FILTER_1,     # 3
    KNOB_UNROLL_FILTER_2,     # 4
    KNOB_UNROLL_FILTER_3,     # 5
    KNOB_UNROLL_COEF_SHIFT_1, # 6
    KNOB_UNROLL_COEF_SHIFT_2, # 7
    KNOB_UNROLL_TOTAL,        # 8
    KNOB_UNROLL_PARALLEL_1,   # 9
    KNOB_UNROLL_PARALLEL_2,   # 10
    KNOB_UNROLL_PARALLEL_3,   # 11
    KNOB_NUM_WORK_ITEMS,      # 12
    KNOB_NUM_WORK_GROUPS,     # 13
    KNOB_SIMD,                # 14
    KNOB_COMPUTE_UNITS))      # 15


# ***************************************************************************


def removeCombinations(combs):

    finalList = []

    for c in combs:
        copyit = True

        if c[11] * c[12] > NUM_FILTERS: copyit = False
        if c[11] * c[12] == 1 and (c[13] != 1 or c[14] != 1): copyit = False
        if c[13] > c[11]: copyit = False
        if c[14] > c[12]: copyit = False
        if c[1] * (math.log(c[4],2)+1) * c[7] * c[13] * c[14] > 12: copyit = False
        
        
        if copyit:
            finalList.append(c)

    return finalList




def main():

    doCreateFolders = 0

    if len(sys.argv) > 1:
        doCreateFolders = int(sys.argv[1])

    finalCombinations = removeCombinations(allCombinations)

    print("Num combinations: " + str(len(finalCombinations)))
    print("vs " + str(len(allCombinations)))



    if doCreateFolders == 1:
        createFolders(
                finalCombinations,
                templateFilepath,
                kernelFilename,
                dirToCopy,
                outRootPath,
                outBasename,
                outKnobFilename,
                logFilename,
				outKernelSubfolder)
    else:
        print("\nNote: To actually create the folders, run:\n" + sys.argv[0] + " 1\n")





if __name__ == "__main__":
    main()






