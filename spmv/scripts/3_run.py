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
# Filename: 3_run.py
# Version: 1.0
# Description: Python script to run the SPMV benchmarks.
# Author: Quentin Gautier


import os
import sys
import shutil
import subprocess

benchmarksFolder = "../benchmarks"

hostGenScript         = "spmv_host_gen.py"
runProgramScript      = "run.sh"
resultsFilename       = "run_results.txt"
outputResultsFilename = "results.csv"


def runScript(script, path):
    subprocess.call("./" + script, cwd=path, shell=True)


def main():

    device = "fpga"

    if len(sys.argv) >= 2:
        device = sys.argv[1]

    print("Using " + device + " device.")
    print("( Usage: " + sys.argv[0] + " <fpga|gpu|gpu_all|cpu|cpu_all> )\n")

    outputFilename = device + "_" + outputResultsFilename

    # Generate host files
    print("Generating host files...")
    runScript(hostGenScript, benchmarksFolder)

    # Run host files
    print("Running programs...")
    runScript(runProgramScript + " " + device, benchmarksFolder)

    # Copy results file to current directory
    shutil.copy(os.path.join(benchmarksFolder, resultsFilename), resultsFilename)

    # Parse results file and output
    print("")
    runScript("parse_results.py " + resultsFilename + " " + outputFilename, ".")

    print("Done.")

if __name__ == "__main__":
    main()






