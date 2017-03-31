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
# Filename: generateDesigns.py
# Version: 1.0
# Description: Python script to create unique designs from a template and combinations of knobs.
# Author: Quentin Gautier


import os.path
import shutil


##############
# README
##############
# This function copies the folder given by 'dirToCopy' into multiple design folders.
# Then it opens and parses the given template knob file to generate proper knobs.
# Then it parses the given kernel file, replaces the "#include" line by the generated knobs,
# and writes the result into the design folders.
#
# The directory to copy should contain links to the host code and the Makefile.
# The directory to copy MUST NOT contain links to the kernel file.
##############

FULL_UNROLL = -1 # A constant for fully unrolling a loop

def createFolders(
        finalCombinations,         # Combinations of knobs
        templateFilepath,          # Knobs template file
        kernelFilename,            # Kernel file to copy 
        dirToCopy,                 # Directory containing the source code                                                                    
        outRootPath,               # Name of output directory where all folders are generated
        outBasename,               # Base name of generated folders
        outKnobFilename,           # Name of generated knob file
        logFilename = "params.log",# Log file to copy useful information
        outKernelSubfolder = ""    # Kernel will be copied into 'outRootPath/outBasenameX/outKernelSubfolder/'
        ):

    logFile = open(logFilename, 'wt')

    # Read kernel file and find include line
    kernel = []
    includeLine = 0
    with open(kernelFilename, "rt") as kin:
        kernel = kin.read().split('\n')
        for i,line in enumerate(kernel):
            if "#include" in line:
                includeLine = i
                break

    # For each combination of knobs
    for (num, values) in enumerate(finalCombinations):

        if(num % 1000) == 0:
            percent = float(num) / len(finalCombinations) * 100.0
            print("[" + str(int(percent)) + "%]")

        strValues = [' ' if v==FULL_UNROLL else str(v) for v in values]

        copiedDir = os.path.join(outRootPath, outBasename) + str(num)
        shutil.copytree(dirToCopy, copiedDir, True)

        outKernelPath = os.path.join(copiedDir, outKernelSubfolder, os.path.basename(kernelFilename))
        outKnobPath   = os.path.join(copiedDir, outKnobFilename)


        # Read template file and set the knob values
        knobs_text = ""
        with open(templateFilepath, "rt") as fin:
            for line in fin:

                newline = line

                for (i, replace) in reversed(list(enumerate(strValues))):
                    text = '%' + str(i+1)
                    newline = newline.replace(text, replace)

                knobs_text += newline

        kernel[includeLine] = knobs_text


        # Write the knob file
        with open(outKnobPath, "wt") as fout:
            fout.write(knobs_text)

        # Write the new kernel file
        with open(outKernelPath, "wt") as kout:
            kout.write("\n".join(kernel))

        # Log design path and knob values
        logFile.write(copiedDir + " " + ' '.join(map(str, values)) + "\n")





