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
# Description: Python script to generate unique designs for the BFS benchmark.
# Author: Quentin Gautier


import math
import itertools
import os.path
import sys
import shutil


inFilepath  = "../src/knobs.h.template" # Knobs template file
dirToCopy   = "../benchmarks/basefolder" # Directory containing the source code
outRootPath = "../benchmarks"            # Name of output directory where all folders are generated

outBasename = "bfs"                      # Base name of generated folders
outFilename = "knobs.h"                  # Name of generated knob file
logFilename = "bfs_params.log"           # Log file to copy useful information


# ***************************************************************************
# Knobs
# ***********

FULL_UNROLL = -1

KNOB_UNROLL_FACTOR     = [1, 4, 6, 8, 10, 12, 16]
KNOB_COMPUTE_UNITS_1   = [1, 2, 4, 6] 
KNOB_COMPUTE_UNITS_2   = [1, 2, 4, 8, 10]
KNOB_SIMD_2            = [2, 4, 8]
KNOB_BRANCH            = [0, 1]
KNOB_MASK_TYPE         = [1, 2, 3]




allCombinations = list(itertools.product(
    KNOB_UNROLL_FACTOR,        # 0
    KNOB_COMPUTE_UNITS_1,      # 1
    KNOB_COMPUTE_UNITS_2,      # 2
    KNOB_SIMD_2,               # 3
    KNOB_BRANCH,               # 4
    KNOB_MASK_TYPE             # 5
    ))

# ***************************************************************************


def removeCombinations(combs):

    finalList = []

    for c in combs:
        copyit = True

        if c[0] >= 8 and c[1] >= 2 : copyit = False
        if c[1] > 1 and c[4] == 0: copyit = False
        
        
        if copyit:
            finalList.append(c)

    return finalList



def createFolders(finalCombinations):

    if not os.path.isfile(inFilepath):
        print("File " + inFilepath + " does not exists.")
        sys.exit(1)


    logFile = open(logFilename, 'wt')


    for (num, values) in enumerate(finalCombinations):

        if(num % 1000) == 0:
            percent = float(num) / len(finalCombinations) * 100.0
            print("[" + str(int(percent)) + "%]")

        strValues = [' ' if v==FULL_UNROLL else str(v) for v in values]

        copiedDir = os.path.join(outRootPath, outBasename) + str(num)
        shutil.copytree(dirToCopy, copiedDir, True)

        outPath = os.path.join(copiedDir, outFilename)

        with open(outPath, "wt") as fout:
            with open(inFilepath, "rt") as fin:
                for line in fin:

                    newline = line

                    for (i, replace) in reversed(list(enumerate(strValues))):

                        text = '%' + str(i+1)
                        newline = newline.replace(text, replace)

                    fout.write(newline)


        logFile.write(copiedDir + " " + ' '.join(map(str, values)) + "\n")



#def writeParams(inFilepath, outFilename, combinations):
#    inFile = open(inFilepath, 'rt')
#    outFile = open(outFilename, 'wt')
#
#
#    for i, line in enumerate(inFile):
#        values = line.split()
#        num = int(values[0])
#
#        comb = combinations[num]
#
#
#        outFile.write(' '.join(values) + ' ' + str(comb) + "\n")





def main():

    doCreateFolders = 0

    if len(sys.argv) > 1:
        doCreateFolders = int(sys.argv[1])

    finalCombinations = removeCombinations(allCombinations)

    print("Num combinations: " + str(len(finalCombinations)))
    print("vs " + str(len(allCombinations)))


    #writeParams("small.txt", "small_params.txt", finalCombinations)
    #writeParams("big.txt", "big_params.txt", finalCombinations)

    if doCreateFolders == 1:
        createFolders(finalCombinations)
    else:
        print("\nNote: To actually create the folders, run:\n" + sys.argv[0] + " 1\n")





if __name__ == "__main__":
    main()






