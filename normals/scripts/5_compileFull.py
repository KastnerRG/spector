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
# Filename: 5_compileFull.py
# Version: 1.0
# Description: Python script to fully compile all the designs with AOCL.
# Author: Quentin Gautier


import subprocess
import os
import multiprocessing as mp
import random
import signal
import sys
import re;


inFilename  = "small.txt" # Design folder list to compile
outFilename = "compileFull.log"         # Logfile of compiled designs

clBasename  = "normals"                 # Basename of OpenCL file


def launchCommand(path):
    
    date = subprocess.check_output("date", cwd=path, shell=True).strip()
    print(date + "  " + path)
    
    command = "time aoc " + clBasename + ".cl"

    subprocess.call(command, cwd=path, shell=True)

    pathDelete = os.path.join(path, clBasename)
    #commandDelete = "rm -R -- */ ; find . -size +2M -delete"
    commandDelete = "rm -R -- */ ; find ! \( -name " + clBasename + ".log -o -name \*.attrib -o -name \*.area -o -name acl_quartus_report.txt -o -name top.fit.summary \) -type f -exec rm {} +"

    subprocess.call(commandDelete, cwd=pathDelete, shell=True)

    outFile = open(outFilename, 'at')
    outFile.write(path + '\n')
    outFile.close()




def main():

    # Get number of processors to use
    num_processes = 0
    if len(sys.argv) >= 2:
        num_processes = int(sys.argv[1])

    if num_processes > 0:
        print("Using " + str(num_processes) + " processes")
    else:
        print("Using all available processes")


    folders = []

    # Get folders from file
    inFile = open(inFilename, 'rt')
    for i, line in enumerate(inFile):
        values = line.split()
        folders.append(values[0])

    # Remove folders that we already compiled
    if(os.path.isfile(outFilename)):
        compiled = []
        logFile = open(outFilename, 'rt')
        for line in logFile:
            name = re.split(r'\s', line)
            compiled.append(name[0])
        logFile.close()

        for f in compiled:
            folders.remove(f)

    
    # Compile in random order
    # (can help to get overview of the design space for partial results)
    random.shuffle(folders)

    # Compile the folders on different processes
    if num_processes > 0:
        pool = mp.Pool(num_processes)
    else:
        pool = mp.Pool()
    result = pool.map_async(launchCommand, folders).get(31536000) # timeout of 365 days





if __name__=="__main__":
    main()
