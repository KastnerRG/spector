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
# Description: Python script to generate unique designs for the histogram benchmark.
# Author: Quentin Gautier


import itertools
import sys

sys.path.append("../../common/scripts")
from generateDesigns import createFolders


templateFilepath = "../src/params.h.template" # Knobs template file
kernelFilename   = "../src/histogram.cl"      # Kernel file to copy 
dirToCopy        = "../benchmarks/basefolder" # Directory containing the source code

outRootPath      = "../benchmarks"            # Name of output directory where all folders are generated
outBasename      = "hist_design"              # Base name of generated folders
outKnobFilename  = "params.h"                 # Name of generated knob file
logFilename      = "params.log"               # Log file to copy useful information

# ***************************************************************************
# Knobs
# ***********


KNOB_NUM_HIST  = [i for i in range(1, 17)]
KNOB_HIST_SIZE = [256, 257]

KNOB_NUM_WORK_ITEMS  = [1, 2, 4, 8, 16, 32, 64]
KNOB_NUM_WORK_GROUPS = [1, 2, 4, 8, 16, 32, 64]

KNOB_SIMD          = [1]
KNOB_COMPUTE_UNITS = [1, 2, 4, 8]

KNOB_ACCUM_SMEM    = [0, 1]

KNOB_UNROLL_FACTOR = [1, 2]



allCombinations = list(itertools.product(
    KNOB_NUM_HIST,        # 0
    KNOB_HIST_SIZE,       # 1
    KNOB_NUM_WORK_ITEMS,  # 2
    KNOB_NUM_WORK_GROUPS, # 3
    KNOB_SIMD,            # 4
    KNOB_COMPUTE_UNITS,   # 5
    KNOB_ACCUM_SMEM,      # 6
    KNOB_UNROLL_FACTOR    # 7
    ))
# ***************************************************************************


def removeCombinations(combs):

    finalList = []

    for c in combs:
        copyit = True

        if c[4] > c[2]: copyit = False
        if c[5] > c[3]: copyit = False
        if c[6] == 1 and (c[3] > 1 or c[2] == 1): copyit = False
        if c[6] == 1 and c[4] > 1: copyit = False
        if c[2] * c[3] > 1 and c[1] == 256 and c[0] > 4: copyit = False

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
                logFilename)
    else:
        print("\nNote: To actually create the folders, run:\n" + sys.argv[0] + " 1\n")





if __name__ == "__main__":
    main()






