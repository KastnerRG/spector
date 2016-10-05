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
# Filename: 6_runPrograms.py
# Version: 1.0
# Description: Python script to run the compiled designs on FPGA.
# Author: Quentin Gautier


import os
import subprocess
import re
import sys


outfilename       = "fpga_results.csv" # Output filename for results

paramsfilename    = "small.txt" # List of design folders with their parameters (knob values)
compiledfilename  = "small.txt" # List of design folders

donefilename      = "fpga_run_done.csv"       # Designs already run

reportfilename    = "bin_tdfir/acl_quartus_report.txt" # Primary report file to get info
reportfilename2   = "bin_tdfir/top.fit.summary"        # Secondary report file in case the first one does not exist

kernel_filenames  = ["bin_tdfir/tdfir.attrib"] # Kernel report files

clBasename        = "bin/tdfir"      # Basename for the OpenCL file
exeFilename       = "bin/tdfir" # Program filename

defaultInput      = "" # Default input file

verif_re = re.compile(r'Verification: (\S+)')  # Regex for verification
verif_text =                          "PASS" # Verification text to check for success
time_re  = re.compile(r'Total time: (\S+) ms') # Regex for running time


num_runs = 5 # Number of times to run the algorithm


def main():

    # Get input file
    progInputFile =  defaultInput
    if len(sys.argv) > 1:
        progInputFile = os.path.join("../", sys.argv[1]) # TODO set path correctly

    # Get files to not run
    done = []
    if os.path.exists(donefilename):
        with open(donefilename, 'rt') as donefile:
            for line in donefile:
                done.append(line.split(",")[0].strip())

    # Get files to run
    dirs = []
    with open(compiledfilename, 'rt') as infile:
        for line in infile:
            value = line.split()[0]
            if not value in done:
                dirs.append(value)

    # Get knob values and other parameters for each design
    params = {}
    with open(paramsfilename, 'rt') as parfile:
        for line in parfile:
            data = line.split()
            params[data[0]] = data[1:]



    outfile = open(outfilename, 'wt')


    print(str(len(dirs)) + " directories")





    # Write CSV header
    outfile.write("ID," + ",".join(["param_"+str(i) for i in range(len(params[dirs[0]]))]) + "," + ",".join(["estim_throughput_" + str(i) for i in range(len(kernel_filenames))])
            + ",time_s,logic,mem,ram,dsp,fmax,verif,error,fit\n")


    # for each directory
    #
    for d in dirs:


        reportfile_path  = os.path.join(d, reportfilename)
        reportfile_path2 = os.path.join(d, reportfilename2)


        verif = 'F'
        time = 999
        error = 'Y'
        fit = 'N'

        logicUse = -1
        memUse = -1 
        ramUse = -1 
        dspUse = -1 
        fmax   = -1 

        estim_throughput = []

        # Compile and run
        #
        for itry in range(4): # this loop is only in case the program fails to run (see below)

            try:
                if os.path.isfile(os.path.join(d, clBasename + ".aocx")):

                    fit = 'Y'

                    subprocess.call("make clean; make fpga > /dev/null 2>&1", cwd=d, shell=True)


                    output = ''
                    if params[d][6] != 1:
                        print("Running " + d)
                        output = subprocess.check_output("timeout 20s ./" + exeFilename + " " + progInputFile + " fpga " + str(num_runs), cwd=d, shell=True, stderr=subprocess.STDOUT) #, timeout=5)


                    for line in output.split(b'\n'):
                        line = str(line)
                        m = verif_re.search(line)
                        m2 = time_re.search(line)

                        if m:
                            verif = 'P' if m.group(1).replace("'","") == verif_text else 'F'

                        if m2:
                            time = float(m2.group(1)) / 1000.0

                    error = 'N'


            # Sometimes the program fails to run, and reprogramming the FPGA manually may fix the problem
            except subprocess.CalledProcessError:
                print("ERROR " + d)
                subprocess.call("aocl program acl0 " + clBasename + ".aocx", cwd=d, shell=True)
                continue
            except:
                continue

            break



        # Read area info
        #
        if os.path.isfile(reportfile_path):

            fin = open(reportfile_path, 'r')

            logic_re = re.compile(r'Logic utilization:\s+(\S+)\s+')
            mem_re = re.compile(r'Memory bits:\s+(\S+)\s+')
            ram_re = re.compile(r'RAM blocks:\s+(\S+)\s+')
            dsp_re = re.compile(r'DSP blocks:\s+(\S+)\s+')
            fmax_re  = re.compile(r'Kernel fmax:\s+(\S+)\s+')


            for line in fin:
                m = logic_re.search(line)
                m2 = mem_re.search(line)
                m3 = ram_re.search(line)
                m4 = dsp_re.search(line)
                m5 = fmax_re.search(line)

                if m:
                    logicUse = int(m.group(1).replace(",",""))

                if m2:
                    memUse = int(m2.group(1).replace(",",""))

                if m3:
                    ramUse = int(m3.group(1).replace(",",""))

                if m4:
                    dspUse = int(m4.group(1).replace(",",""))

                if m5:
                    fmax   = float(m5.group(1).replace(",",""))


        elif os.path.isfile(reportfile_path2):

            fin = open(reportfile_path2, 'r')

            logic_re = re.compile(r'Logic utilization.*:\s+(\S+)\s+')
            mem_re = re.compile(r'Total block memory bits :\s+(\S+)\s+')
            dsp_re = re.compile(r'Total DSP Blocks :\s+(\S+)\s+')


            for line in fin:
                m = logic_re.search(line)
                m2 = mem_re.search(line)
                m4 = dsp_re.search(line)

                if m:
                    logicUse = int(m.group(1).replace(",",""))

                if m2:
                    memUse = int(m2.group(1).replace(",",""))

                if m4:
                    dspUse = int(m4.group(1).replace(",",""))

                else:
                    pass

        # Read estimated throughput
        #
        through_re = re.compile(r'Throughput:\s+(\S+)\s+')
        for name in kernel_filenames:
            k_path = os.path.join(d, name)
            has_throughput = False
            if os.path.isfile(k_path):
                with open(k_path, 'rt') as f:
                    for line in f:
                        m = through_re.search(line)
                        if m:
                            estim_throughput.append(m.group(1))
                            has_throughput = True
                            break
            if not has_throughput:
                estim_throughput.append("0")


        # Write result to CSV
        #
        #csv = d + "," + str(time) + "," + verif + "," + error + "," + fit
        csv = d + "," + ",".join(params[d]) + "," + ",".join(estim_throughput) + "," + str(time) + "," + str(logicUse) + "," + str(memUse) + "," + str(ramUse) + "," + str(dspUse) + "," + str(fmax) + "," + verif + "," + error + "," + fit

        outfile.write(csv + "\n")
        outfile.flush()

        print(csv)









if __name__=="__main__":
    main()


