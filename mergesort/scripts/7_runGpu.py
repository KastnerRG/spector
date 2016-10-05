#!/usr/bin/python3

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
# Filename: 7_runGpu.py
# Version: 1.0
# Description: Python script to run the designs on GPU.
# Author: Quentin Gautier


import os
import subprocess
import re
import sys


outfilename       = "gpu_results.csv"          # Result filename

paramsfilename    = "small.txt"                # List of designs with their parameters
compiledfilename  = "small.txt"                # List of designs to run

donefilename      = "gpu_run_done.csv"         # List of designs to not run

clBasename        = "mergesort"                # Basename for the OpenCL file
exeFilename       = "merge"                    # Program filename

defaultInput      = ""                         # Default input file

verif_re = re.compile(r'Verification: (\S+)')  # Regex for verification
verif_text =                          "Passed" # Verification text to check for success
time_re  = re.compile(r'Total time: (\S+) ms') # Regex for running time

num_runs = 5



def main():

    # Get input file
    progInputFile =  defaultInput
    if len(sys.argv) > 1:
        progInputFile = os.path.join("../", sys.argv[1]) # TODO set path correctly


    done = []
    if os.path.exists(donefilename):
        with open(donefilename, 'rt') as donefile:
            for line in donefile:
                done.append(line.split(",")[0].strip())


    dirs = []
    with open(compiledfilename, 'rt') as infile:
        for line in infile:
            value = line.split()[0]
            if not value in done:
                dirs.append(value)

    params = {}
    with open(paramsfilename, 'rt') as parfile:
        for line in parfile:
            data = line.split()
            params[data[0]] = data[1:]



    outfile = open(outfilename, 'wt')


    print(str(len(dirs)) + " directories")



    
    
    
    outfile.write("ID," + ",".join(["param_"+str(i) for i in range(len(params[dirs[0]]))]) + ","
            + ",time,verif,error,fit\n")


    # for each directory
    #
    for d in dirs:


        verif = 'F'
        time = 999
        error = 'Y'
        fit = 'N'


        # Compile and run
        #
        for itry in range(4):

            try:
                #if os.path.isfile(os.path.join(d, clBasename + ".aocx")):
                if True:
                          
                    fit = 'Y'
                             
                    subprocess.call("make gpu > /dev/null 2>&1", cwd=d, shell=True)
                  
                    print("Running " + d)
                    output = subprocess.check_output("./" + exeFilename + " " + progInputFile + " gpu " + str(num_runs), cwd=d, shell=True, stderr=subprocess.STDOUT) #, timeout=5)


                    for line in output.split(b'\n'):
                        line = str(line)
                        m = verif_re.search(line)
                        m2 = time_re.search(line)

                        if m:
                            verif = 'P' if m.group(1).replace("'","") == verif_text else 'F'
                        
                        if m2:
                            time = float(m2.group(1))

                    error = 'N'
                 

            except subprocess.CalledProcessError:
                print("ERROR " + d)
                continue
            except:
                continue

            break




        csv = d + "," + ",".join(params[d]) + "," + "," + str(time) + "," + verif + "," + error + "," + fit
        
        outfile.write(csv + "\n")
        outfile.flush()

        print(csv)









if __name__=="__main__":
	main()


