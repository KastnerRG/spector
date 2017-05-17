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
# Filename: 4_parseEstimations.py
# Version: 1.0
# Description: Python script to parse the AOC estimated results and classify these results.
# Author: Quentin Gautier


import os.path
import re
import sys


aocFilename     = "bfs_fpga/bfs_fpga.log"   # Altera OpenCL log filename (relative to design folder)

smallFilename   = "small.txt" # File to store small designs (likely to fit)
bigFilename     = "big.txt"   # File to store large designs (not likely to fit)

foldersFilename = "params.log"          # Log file containing design folder list




smallFile = open(smallFilename, 'wt')
bigFile = open(bigFilename, 'wt')


numWarningFiles = 0


folderFile = open(foldersFilename, 'rt')

for line in folderFile:

	i = line.split()[0]
	filePath = os.path.join(i, aocFilename)

	if not os.path.isfile(filePath):
		print(filePath + " does not exist")
		#sys.exit(1)
		continue



	fin = open(filePath, 'r')

	logic_re = re.compile(r'Logic utilization\s+;\s*(\d+)%')
	reg_re   = re.compile(r'Dedicated logic registers\s+;\s*(\d+)%')
	mem_re   = re.compile(r'Memory blocks\s+;\s*(\d+)%')
	dsp_re   = re.compile(r'DSP blocks\s+;\s*(\d+)%')
	warn_re  = re.compile(r'Warning')


	logicUse = 0
	regUse = 0
	memUse = 0
	dspUse = 0

	hasWarning = False

	for line2 in fin:
		m = logic_re.search(line2)
		m2 = reg_re.search(line2)
		m3 = mem_re.search(line2)
		m4 = dsp_re.search(line2)

		if m:
			logicUse = int(m.group(1))

		if m2:
			regUse = int(m2.group(1))
			
		if m3:
			memUse = int(m3.group(1))

		if m4:
			dspUse = int(m4.group(1))

		if warn_re.search(line2):
			hasWarning = True

	if logicUse * regUse * memUse == 0: # * dspUse == 0:
		print(filePath)
		continue

	text = line.strip() + " " + ' '.join(map(str, [logicUse, regUse, memUse, dspUse])) + '\n'


	if logicUse > 85 or dspUse > 100 or regUse > 100 or memUse > 100:
		bigFile.write(text)
	else:	
		smallFile.write(text)
		if hasWarning: numWarningFiles += 1


print(str(numWarningFiles) + " warnings among smalls")

