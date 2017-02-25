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
# Filename: logic_util_extract.py
# Version: 1.0
# Description: Python script to extract the logic utilization of the designs.
# Author: Pingfan Meng



import os.path


blockdim_x_pool=[8,16,32,64]
blockdim_y_pool=[8,16,32,64]
simd_loc_pool=[1,2,4,8]
simd_loc_pool2=[2,4,8]
simd_type_pool=[0,1]
block_size_f_pool=[1,2,4]
block_unroll_pool=[0,1]

dct_unroll_pool=[0,1]

simd_wi_pool=[1,2,4,8]
comp_u_pool=[1,2]

output_file=open('logic_util.txt','w')
out_string=''

for blockdim_x in blockdim_x_pool:
    for blockdim_y in blockdim_y_pool:
        for simd_type in simd_type_pool:
            if simd_type==0:
                for simd_loc in simd_loc_pool:
                    for block_size_f in block_size_f_pool:
                        for block_unroll in block_unroll_pool:
                            for dct_unroll in dct_unroll_pool:
                                for simd_wi in simd_wi_pool:
                                    for comp_u in comp_u_pool:
                                        if (blockdim_x*blockdim_y<=2094) and (blockdim_y%(block_size_f*8)==0) and (blockdim_x%(block_size_f*8)==0) and (blockdim_x%simd_loc==0) and ((block_unroll==0)or(block_size_f!=1)):
                                            
                                            input_file_name_string = ('dct_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'simdType'+str(simd_type)
                                                    +'_'+'simdLoc'+str(simd_loc)+'_'+'blockSizeF'+str(block_size_f)+'_'+'blockU'+str(block_unroll)+'_'
                                                    +'DCTU'+str(dct_unroll)+'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'_rpt_name/acl_quartus_report.txt')


                                            if os.path.isfile(input_file_name_string):
                                                print('Open '+input_file_name_string)
                                                input_file=open(input_file_name_string,'r')

                                                tmp_string=input_file.readline()
                                                flag=-1

                                                while(tmp_string!=''):

                                                    if (tmp_string[0:18]=='Logic utilization:'):
                                                        print "found"
                                                        tmp_count=0
                                                        while(tmp_string[tmp_count]!='/'):
                                                            tmp_count=tmp_count+1
                                                        data_string=tmp_string[18:tmp_count]
                                                        data_string=data_string.replace(',','')



                                                        out_string = (out_string+str(blockdim_x)+','+str(blockdim_y)+','+ str(simd_type)
                                                                +','+str(simd_loc)+','+ str(block_size_f)+','+ str(block_unroll)+','+ str(dct_unroll)+','+str(simd_wi)+','+str(comp_u)+','+data_string+'\n')

                                                    tmp_string=input_file.readline()

                                                input_file.close()


            else:
                for simd_loc in simd_loc_pool2:
                    for block_size_f in block_size_f_pool:
                        for block_unroll in block_unroll_pool:
                            for dct_unroll in dct_unroll_pool:
                                for simd_wi in simd_wi_pool:
                                    for comp_u in comp_u_pool:
                                        if (blockdim_x*blockdim_y<=2094) and (blockdim_y%(block_size_f*8*simd_loc)==0)  and (blockdim_x%(block_size_f*8)==0) and ((block_unroll==0)or(block_size_f!=1)):

                                            input_file_name_string = ('dct_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'simdType'+str(simd_type)
                                                    +'_'+'simdLoc'+str(simd_loc)+'_'+'blockSizeF'+str(block_size_f)+'_'+'blockU'+str(block_unroll)+'_'+'DCTU'+str(dct_unroll)
                                                    +'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'_rpt_name/acl_quartus_report.txt')


                                            if os.path.isfile(input_file_name_string):
                                                print('Open '+input_file_name_string)
                                                input_file=open(input_file_name_string,'r')

                                                tmp_string=input_file.readline()
                                                flag=-1

                                                while(tmp_string!=''):

                                                    if (tmp_string[0:18]=='Logic utilization:'):
                                                        tmp_count=0
                                                        while(tmp_string[tmp_count]!='/'):
                                                            tmp_count=tmp_count+1
                                                        data_string=tmp_string[18:tmp_count]
                                                        data_string=data_string.replace(',','')

                                                        out_string = (out_string+str(blockdim_x)+','+str(blockdim_y)+','+ str(simd_type)
                                                                +','+str(simd_loc)+','+ str(block_size_f)+','+ str(block_unroll)+','+ str(dct_unroll)+','+str(simd_wi)+','+str(comp_u)+','+data_string+'\n')

                                                    tmp_string=input_file.readline()

                                                input_file.close()

output_file.write(out_string)
output_file.close()

