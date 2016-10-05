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
# Filename: spmv_cl_gen.py
# Version: 1.0
# Description: Python script to generate the OpenCL designs.
# Author: Pingfan Meng

blockdim_pool=[1,2,4,8,16,32,64,128,256,512]
comp_u_pool=[1,2,3,4]
vect_w_pool=[1,2,4,8,16]
unroll_f_pool=[1,2,4,8,16,32]#range(1,17)

base_file_name='spmv_base.cl'


for blockdim in blockdim_pool:
        for comp_u in comp_u_pool:
                for unroll_f in unroll_f_pool:
                        for vect_w in vect_w_pool:
                                if (comp_u*unroll_f*vect_w<=64):
                                        cl_file_name='spmv_'+'b'+str(blockdim)+'_'+'compu'+str(comp_u)+'_'+'unrollf'+str(unroll_f)+'_'+'vectw'+str(vect_w)+'.cl'
                                        input_base_file=open(base_file_name,'r')
                                        output_cl_file=open(cl_file_name,'w')
                                        kernel_string='#define BLOCKDIM '+str(blockdim)+'\n\n'
                                        kernel_string=kernel_string+'#define COMP_U '+str(comp_u)+'\n\n'
                                        kernel_string=kernel_string+'#define UNROLL_F '+str(unroll_f)+'\n\n'
                                        kernel_string=kernel_string+'#define VECT_WIDTH '+str(vect_w)+'\n\n'
                
                                        tmp_string=input_base_file.readline()

                                        while (tmp_string!=''):
                                                kernel_string=kernel_string+tmp_string
                                                tmp_string=input_base_file.readline()
                        
                                        input_base_file.close()
                                        output_cl_file.write(kernel_string)
                                        output_cl_file.close()
