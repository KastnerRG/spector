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
# Filename: 1_runGPU.py
# Version: 1.0
# Description: Python script to run the designs for the DCT benchmark.
# Author: Quentin Gautier


import os
import sys
import signal
import shutil
import subprocess


srcFolder = "../../src/gpu"
scriptsFolder    = "."
benchmarksFolder = "../../benchmarks"

hostGenScript = "dct_host_gen_GPU.py"
clGenScript   = "../dct_cl_gen.py"
hostRunScript = "runGPU.sh"
#logicExtractScript  = "logic_util_extract.py"
timingExtractScript = "timing_extract.py"



def runScript(script, path):
    pro = subprocess.Popen("./" + script, cwd=path, shell=True, executable="/bin/bash")
    try:
        pro.wait()
    except:
       os.killpg(os.getpgid(pro.pid), signal.SIGTERM) 

    
def copyFile(f, src, dst):
    shutil.copy(
            os.path.join(src, f),
            os.path.join(dst, os.path.basename(f)))


def main():

    print("Copying files...")

    # Copy source files
    for f in os.listdir(srcFolder):
        if os.path.isfile(os.path.join(srcFolder, f)):
            shutil.copy(
                    os.path.join(srcFolder, f),
                    os.path.join(benchmarksFolder, f))

    # Copy scripts
    for s in [hostGenScript, clGenScript, hostRunScript, timingExtractScript]:
        copyFile(s, scriptsFolder, benchmarksFolder)

    # Generate CPP/CL files
    print("Generating cpp/cl files...")
    runScript(os.path.basename(clGenScript), benchmarksFolder)
    runScript(os.path.basename(hostGenScript), benchmarksFolder)
    
    
    # Run all programs
    print("Running programs...")
    runScript(os.path.basename(hostRunScript), benchmarksFolder)


    # Extract data
    print("Reading results...")
    #runScript(os.path.basename(logicExtractScript), benchmarksFolder)
    runScript(os.path.basename(timingExtractScript), benchmarksFolder)

    print("Done.")

if __name__ == "__main__":
    main()






