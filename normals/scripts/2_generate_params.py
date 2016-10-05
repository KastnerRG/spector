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


import math
import itertools
import os.path
import sys
import shutil


##############
# README
##############
# This script copies the folder given by 'dirToCopy' into multiple design folders.
# Then it opens and parses the given template knob file.
# Then it writes the multiple resulting knob files into the design folders.
# Then it opens the given kernel file, reads it into memory, and writes a new kernel file in the design folders with:
#    - At the top, the content of the knob file
#    - followed by the content of the original kernel without its first line.
#
# The directory to copy should contain links to the host code and the Makefile.
# The directory to copy MUST NOT contain links to the knob file or kernel file.
##############


inFilepath     = "../src/knobs.h.template"  # Knobs template file
kernelFilename = "../src/normals.cl"        # Kernel file to copy *without the first line* 
dirToCopy      = "../benchmarks/basefolder" # Directory containing the source code
outRootPath    = "../benchmarks"            # Name of output directory where all folders are generated

outBasename    = "normals_design"           # Base name of generated folders
outFilename    = "knobs.h"                  # Name of generated knob file
logFilename    = "params.log"               # Log file to copy useful information


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
        outKernelPath = os.path.join(copiedDir, os.path.basename(kernelFilename))


        # Read kernel file and remove the first line
        kernelToCopy = ""
        with open(kernelFilename, "rt") as kin:
            kernel = kin.read().split('\n')[1:]
            kernelToCopy = '\n'.join(kernel)



        with open(outPath, "wt") as fout, open(outKernelPath, "wt") as kout:
            with open(inFilepath, "rt") as fin:
                for line in fin:

                    newline = line

                    for (i, replace) in reversed(list(enumerate(strValues))):

                        text = '%' + str(i+1)
                        newline = newline.replace(text, replace)

                    fout.write(newline)
                    kout.write(newline)

                kout.write(kernelToCopy)


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






