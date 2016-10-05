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
# Filename: spmv_host_gen.py
# Version: 1.0
# Description: Python script to generate host programs that run the OpenCL designs.
# Author: Pingfan Meng


blockdim_pool=[1,2,4,8,16,32,64,128,256,512]
comp_u_pool=[1,2,3,4]
vect_w_pool=[1,2,4,8,16]
unroll_f_pool=[1,2,4,8,16,32]#range(1,17)

base_file_name='spmv_host_base.cpp'


for blockdim in blockdim_pool:
        for comp_u in comp_u_pool:
                for unroll_f in unroll_f_pool:
                        for vect_w in vect_w_pool:
                                if (comp_u*unroll_f*vect_w<=64):
                                        cl_file_name='spmv_'+'b'+str(blockdim)+'_'+'compu'+str(comp_u)+'_'+'unrollf'+str(unroll_f)+'_'+'vectw'+str(vect_w)+'.cpp'
                                        input_base_file=open(base_file_name,'r')
                                        output_cl_file=open(cl_file_name,'w')
                                        host_string='#define BLOCKDIM '+str(blockdim)+'\n\n'
                                        host_string=host_string+'#define print_rsl printf("dse result:\\n %d, %d, %d, %d, %f\\n",'+str(blockdim)+','+ str(comp_u)+','+str(unroll_f)+','+str(vect_w)+', run_time_total/NUM_REP)\n\n'

                                        host_string=host_string+'#define CL_FILE_NAME "spmv_b'+str(blockdim)+'_compu'+ str(comp_u)+'_unrollf'+str(unroll_f)+'_vectw'+str(vect_w)+'.aocx"\n\n'

                
                                        tmp_string=input_base_file.readline()

                                        while (tmp_string!=''):
                                                host_string=host_string+tmp_string
                                                tmp_string=input_base_file.readline()
                        
                                        input_base_file.close()
                                        output_cl_file.write(host_string)
                                        output_cl_file.close()
