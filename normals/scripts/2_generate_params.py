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
# Description: Python script to generate unique designs for the normal
#              estimation benchmark.
# Author: Quentin Gautier

import itertools
import sys

sys.path.append("../../common/scripts")
from generateDesigns import createFolders


templateFilepath = "../src/knobs.h.template"  # Knobs template file
kernelFilename   = "../src/normals.cl"        # Kernel file to copy
dirToCopy        = "../benchmarks/basefolder" # Directory containing the source code

outRootPath     = "../benchmarks"            # Name of output directory where all folders are generated
outBasename     = "normals_design"           # Base name of generated folders
outKnobFilename = "knobs.h"                  # Name of generated knob file
logFilename     = "params.log"               # Log file to copy useful information


# ***************************************************************************
# Knobs
# ***********

FULL_UNROLL = -1


KNOB_WORK_ITEMS    = [1, 2, 4, 8]
KNOB_WORK_GROUPS   = [1, 2, 4, 8]

KNOB_COMPUTE_UNITS = [1, 2, 3]

KNOB_UNROLL        = [1, 2, 4, 8]
KNOB_UNROLL_INNER  = [1, 2, 4, 8]

KNOB_WINDOW_SIZE_X = [1, 2, 4, 8]

KNOB_DESIGN_TYPE   = [0, 1]



allCombinations = list(itertools.product(
    KNOB_WORK_ITEMS   ,  # 0 
    KNOB_WORK_GROUPS  ,  # 1 
    KNOB_COMPUTE_UNITS,  # 2 
    KNOB_UNROLL       ,  # 3 
    KNOB_UNROLL_INNER ,  # 4 
    KNOB_WINDOW_SIZE_X,  # 5 
    KNOB_DESIGN_TYPE     # 6
    ))

# ***************************************************************************


def removeCombinations(combs):

    finalList = []

    for c in combs:
        copyit = True

        if c[2] > c[1]: copyit = False
        if c[4] > c[5]: copyit = False
        if c[0] * c[1] > 32: copyit = False
        if c[6] == 1 and c[0] != c[5]: copyit = False
        
        
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






