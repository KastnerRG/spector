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
# Filename: 3_estimateResults.py
# Version: 1.0
# Description: Python script to run AOC estimations on all the designs.
# Author: Quentin Gautier


import subprocess
import os
import multiprocessing as mp
import random
import sys


logName      = "estimateResults.log" # Where to store the results

rootDir      = "../benchmarks"       # Directory containing all the designs
dir_basename = "normals_design"      # Basename of the design folders
cl_basename  = "normals"             # Basename of the OpenCL file


def launchCommand(path):
    command = "aoc " + cl_basename + ".cl -c -g"
    
    subprocess.call(command, cwd=path, shell=True)
    
    pathDelete = os.path.join(path, cl_basename)
    commandDelete = "rm -R -- */ ; find ! \( -name " + cl_basename + ".log \) -type f -exec rm {} + ; rm ../" + cl_basename + ".aoco"
    
    subprocess.call(commandDelete, cwd=pathDelete, shell=True)
    
    outFile = open(logName, 'at')
    outFile.write(path + '\n')
    outFile.close()


	

def main():

    # Get number of processors
    num_processes = 0
    if len(sys.argv) >= 2:
    	num_processes = int(sys.argv[1])
    
    if num_processes > 0:
    	print("Using " + str(num_processes) + " processes")
    else:
    	print("Using all available processes")
    
    
    # Folders to compile
    folders = [os.path.join(rootDir, d) for d in os.listdir(rootDir) if d.startswith(dir_basename)]
    

    # Get folders already compiled from previous log file
    compiled = []
    
    if os.path.isfile(logName):
    	print("Reading processed folders from log file")
    	logFile = open(logName, 'rt')
    	for line in logFile:
    		name = line.split('\n')
    		compiled.append(name[0])
    	logFile.close()
    
    
    for f in compiled:
    	folders.remove(f)
    
    
    
    # Start processes 
    print("Processing " + str(len(folders)) + " folders")

    if num_processes > 0:
    	pool = mp.Pool(num_processes)
    else:
    	pool = mp.Pool()
    pool.map(launchCommand, folders)



if __name__=="__main__":
	main()
