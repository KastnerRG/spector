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
# Filename: 2_run.py
# Version: 1.0
# Description: Python script to run the designs for the DCT benchmark.
# Author: Quentin Gautier


import os
import sys
import shutil
import subprocess


scriptsFolder    = "."
benchmarksFolder = "../benchmarks"

hostGenScript = "dct_host_gen_FPGA.py"
hostRunScript = "run.sh"
logicExtractScript  = "logic_util_extract.py"
timingExtractScript = "timing_extract.py"



def runScript(script, path):    
    subprocess.call("./" + script, cwd=path, shell=True)
    
def runScriptMap(scriptpath):
    runScript(scriptpath[0], scriptpath[1])

def copyFile(f, src, dst):
    shutil.copy(
            os.path.join(src, f),
            os.path.join(dst, f))


def main():

    print("Copying files...")

    # Copy scripts
    for s in [hostGenScript, hostRunScript, logicExtractScript, timingExtractScript]:
        copyFile(s, scriptsFolder, benchmarksFolder)

    # Generate CPP files
    print("Generating cpp files...")
    runScript(hostGenScript, benchmarksFolder)
    
    
    # Run all programs
    print("Running programs...")
    runScript(hostRunScript, benchmarksFolder)

    # Extract data
    print("Reading results...")
    runScript(logicExtractScript, benchmarksFolder)
    runScript(timingExtractScript, benchmarksFolder)

    print("Done.")

if __name__ == "__main__":
    main()






