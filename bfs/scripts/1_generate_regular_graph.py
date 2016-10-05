#!/usr/bin/env python

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
# Filename: 1_generate_regular_graph.py
# Version: 1.0
# Description: Python script to generate a random graph for the BFS benchmark.
# Author: Quentin Gautier


import sys
import random
import networkx as nx


#------------------------------------
def writeFile(graph, filename):
    with open(filename, 'w') as f:
        num_nodes = graph.number_of_nodes()

        f.write(str(num_nodes) + "\n")

        start = 0
        for i in range(num_nodes):
            degree = graph.degree(i)
            f.write(str(start) + " " + str(degree) + "\n")
            start += degree

        f.write("0\n")
        f.write(str(start) + "\n")

        for i in range(num_nodes):
            edges = graph.edges(i)
            for (_,dest) in edges:
                f.write(str(dest) + " " + str(random.randint(1,100)) + "\n")
        
#------------------------------------

    
#------------------------------------
def printUsage(program):
    print("")
    print("Usage: " + program + " <degree> <num_nodes> <output.txt>")
    print("")
#------------------------------------


#------------------------------------
def main():

    if(len(sys.argv) < 4):
        printUsage(sys.argv[0])
        exit()

    
    # Size 
    degree = int(sys.argv[1])

    num_nodes = int(sys.argv[2])

    # Generate
    graph = nx.random_regular_graph(degree, num_nodes)

    # Write to file
    writeFile(graph, sys.argv[3])

#------------------------------------






if __name__ == "__main__":
    main()

