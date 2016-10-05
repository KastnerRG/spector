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
# Filename: sobel_sw_host_code_gen.py
# Version: 1.0
# Description: Python script to generate host programs that run the OpenCL designs.
# Author: Pingfan Meng



W=1920
H=1080

blockdim_x_pool=[2,4,8,16,32]
blockdim_y_pool=[2,4,8,16,32]
subdim_x_pool=[1,2,4,8,16]
subdim_y_pool=[1,2,4,8]
simd_x_pool=[1,2,4,8]
simd_y_pool=[1,2,4,8]

simd_wi_pool=[1,2,4,8]
comp_u_pool=[1,2,4]

param_setting=[]

#subdim_x simd_y
simd_y=1
subdim_x=1

for comp_u in comp_u_pool:
    for simd_wi in simd_wi_pool:
        for simd_x in simd_x_pool:
            for subdim_y in subdim_y_pool:
                for blockdim_x in blockdim_x_pool:
                    for blockdim_y in blockdim_y_pool:
                        if W%(blockdim_x*subdim_x*simd_x)==0 and H%(blockdim_y*subdim_y*simd_y)==0 and blockdim_x%simd_wi==0:
                            rep_flag=0
                            for tmp_setting in param_setting:
                                if tmp_setting==[blockdim_x, blockdim_y, subdim_x, subdim_y, simd_x, simd_y, simd_wi, comp_u]:
                                    rep_flag=1

                            if rep_flag==0:
                                param_setting.append([blockdim_x, blockdim_y, subdim_x, subdim_y, simd_x, simd_y, simd_wi, comp_u])
                                
                                
                                base_file_name='sobel_base.cpp'
                                

                                input_base_file=open(base_file_name,'r')
                                source_file_name='sobel_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'subx'+str(subdim_x)+'_'+'suby'+str(subdim_y)+'_'+'simdx'+str(simd_x)+'_'+'simdy'+str(simd_y)+'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'.cpp'
                                output_source_file=open(source_file_name,'w') 



                                source_string='#define W '+str(W)+'\n'
                                source_string=source_string+'#define H '+str(H)+'\n\n'
                                source_string=source_string+'#define BLOCKDIM_X '+str(blockdim_x)+'\n'
                                source_string=source_string+'#define BLOCKDIM_Y '+str(blockdim_y)+'\n\n'
                                source_string=source_string+'#define SUBDIM_X '+str(subdim_x)+'\n'
                                source_string=source_string+'#define SUBDIM_Y '+str(subdim_y)+'\n\n'
                                source_string=source_string+'#define SIMD_X '+str(simd_x)+'\n'
                                source_string=source_string+'#define SIMD_Y '+str(simd_y)+'\n\n'

                                source_string=source_string+'#define CL_FILE_NAME '+'"sobel_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'subx'+str(subdim_x)+'_'+'suby'+str(subdim_y)+'_'+'simdx'+str(simd_x)+'_'+'simdy'+str(simd_y)+'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'.cl"\n\n'


                                tmp_string=input_base_file.readline()
                                
                                while (tmp_string!=''):
                                    source_string=source_string+tmp_string
                                    tmp_string=input_base_file.readline()

                                input_base_file.close()
                                output_source_file.write(source_string)
                                output_source_file.close()
                                
                                    
#subdim_y simd_x
simd_x=1
subdim_y=1
for comp_u in comp_u_pool:
    for simd_wi in simd_wi_pool:
        for simd_y in simd_y_pool:
            for subdim_x in subdim_x_pool:
                for blockdim_x in blockdim_x_pool:
                    for blockdim_y in blockdim_y_pool:
                        if W%(blockdim_x*subdim_x*simd_x)==0 and H%(blockdim_y*subdim_y*simd_y)==0 and blockdim_x%simd_wi==0:
                            rep_flag=0
                            for tmp_setting in param_setting:
                                if tmp_setting==[blockdim_x, blockdim_y, subdim_x, subdim_y, simd_x, simd_y, simd_wi, comp_u]:
                                    rep_flag=1

                            if rep_flag==0:
                                param_setting.append([blockdim_x, blockdim_y, subdim_x, subdim_y, simd_x, simd_y, simd_wi, comp_u])
                                
                                
                                base_file_name='sobel_base.cpp'
                               

                                input_base_file=open(base_file_name,'r')
                                source_file_name='sobel_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'subx'+str(subdim_x)+'_'+'suby'+str(subdim_y)+'_'+'simdx'+str(simd_x)+'_'+'simdy'+str(simd_y)+'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'.cpp'
                                output_source_file=open(source_file_name,'w') 



                                source_string='#define W '+str(W)+'\n'
                                source_string=source_string+'#define H '+str(H)+'\n\n'
                                source_string=source_string+'#define BLOCKDIM_X '+str(blockdim_x)+'\n'
                                source_string=source_string+'#define BLOCKDIM_Y '+str(blockdim_y)+'\n\n'
                                source_string=source_string+'#define SUBDIM_X '+str(subdim_x)+'\n'
                                source_string=source_string+'#define SUBDIM_Y '+str(subdim_y)+'\n\n'
                                source_string=source_string+'#define SIMD_X '+str(simd_x)+'\n'
                                source_string=source_string+'#define SIMD_Y '+str(simd_y)+'\n\n'

                                source_string=source_string+'#define CL_FILE_NAME '+'"sobel_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'subx'+str(subdim_x)+'_'+'suby'+str(subdim_y)+'_'+'simdx'+str(simd_x)+'_'+'simdy'+str(simd_y)+'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'.cl"\n\n'


                            
                                
                                tmp_string=input_base_file.readline()
                                
                                while (tmp_string!=''):
                                    source_string=source_string+tmp_string
                                    tmp_string=input_base_file.readline()

                                input_base_file.close()
                                output_source_file.write(source_string)
                                output_source_file.close()
