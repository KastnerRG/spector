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
# Filename: dct_host_gen.py
# Version: 1.0
# Description: Python script to generate host programs that run the OpenCL designs.
# Author: Pingfan Meng



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

base_file_name='dct_base.cpp'

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
											input_base_file=open(base_file_name,'r')
                                							cl_file_name='dct_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'simdType'+str(simd_type)+'_'+'simdLoc'+str(simd_loc)+'_'+'blockSizeF'+str(block_size_f)+'_'+'blockU'+str(block_unroll)+'_'+'DCTU'+str(dct_unroll)+'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'.cl'
											cpp_file_name='dct_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'simdType'+str(simd_type)+'_'+'simdLoc'+str(simd_loc)+'_'+'blockSizeF'+str(block_size_f)+'_'+'blockU'+str(block_unroll)+'_'+'DCTU'+str(dct_unroll)+'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'.cpp'
                                							output_cl_file=open(cpp_file_name,'w')
											source_string='#define COMP_U '+str(comp_u)+'\n\n'
											source_string=source_string+'#define SIMD_WI '+str(simd_wi)+'\n\n'
											source_string=source_string+'#define BLOCK_SIZE_F '+str(block_size_f)+'\n\n'
											source_string=source_string+'#define BLOCKDIM_X '+str(blockdim_x)+'\n\n'
											source_string=source_string+'#define BLOCKDIM_Y '+str(blockdim_y)+'\n\n'
											source_string=source_string+'#define SIMD_TYPE '+str(simd_type)+'\n\n'
											source_string=source_string+'#define SIMD_LOC '+str(simd_loc)+'\n\n'
											source_string=source_string+'#define BLOCK_UNROLL '+str(block_unroll)+'\n\n'
											source_string=source_string+'#define DCT_UNROLL '+str(dct_unroll)+'\n\n'
											source_string=source_string+'#define CL_FILE_NAME '+'"'+cl_file_name+'"' +'\n\n'

											source_string=source_string+'#define print_rsl printf("dse result: %d, %d, %d, %d, %d, %d, %d, %d, %d, %f\\n",'+str(blockdim_x)+','+str(blockdim_y)+','+ str(simd_type)+','+str(simd_loc)+','+ str(block_size_f)+','+ str(block_unroll)+','+ str(dct_unroll)+','+str(simd_wi)+','+str(comp_u)+', ELAPSED_TIME_MS(1, 0)/NUM_ITER)\n\n'


											tmp_string=input_base_file.readline()
                                
                                							while (tmp_string!=''):
                                    								source_string=source_string+tmp_string
                                    								tmp_string=input_base_file.readline()

                                							input_base_file.close()
                                							output_cl_file.write(source_string)
                                							output_cl_file.close()
										
			else:
				for simd_loc in simd_loc_pool2:
					for block_size_f in block_size_f_pool:
						for block_unroll in block_unroll_pool:
							for dct_unroll in dct_unroll_pool:
								for simd_wi in simd_wi_pool:
									for comp_u in comp_u_pool:
										if (blockdim_x*blockdim_y<=2094) and (blockdim_y%(block_size_f*8*simd_loc)==0) and (blockdim_x%(block_size_f*8)==0) and ((block_unroll==0)or(block_size_f!=1)):
											input_base_file=open(base_file_name,'r')
                                							cl_file_name='dct_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'simdType'+str(simd_type)+'_'+'simdLoc'+str(simd_loc)+'_'+'blockSizeF'+str(block_size_f)+'_'+'blockU'+str(block_unroll)+'_'+'DCTU'+str(dct_unroll)+'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'.cl'
											cpp_file_name='dct_'+'bx'+str(blockdim_x)+'_'+'by'+str(blockdim_y)+'_'+'simdType'+str(simd_type)+'_'+'simdLoc'+str(simd_loc)+'_'+'blockSizeF'+str(block_size_f)+'_'+'blockU'+str(block_unroll)+'_'+'DCTU'+str(dct_unroll)+'_'+'simdwi'+str(simd_wi)+'_'+'compu'+str(comp_u)+'.cpp'
                                							output_cl_file=open(cpp_file_name,'w')
											source_string='#define COMP_U '+str(comp_u)+'\n\n'
											source_string=source_string+'#define SIMD_WI '+str(simd_wi)+'\n\n'
											source_string=source_string+'#define BLOCK_SIZE_F '+str(block_size_f)+'\n\n'
											source_string=source_string+'#define BLOCKDIM_X '+str(blockdim_x)+'\n\n'
											source_string=source_string+'#define BLOCKDIM_Y '+str(blockdim_y)+'\n\n'
											source_string=source_string+'#define SIMD_TYPE '+str(simd_type)+'\n\n'
											source_string=source_string+'#define SIMD_LOC '+str(simd_loc)+'\n\n'
											source_string=source_string+'#define BLOCK_UNROLL '+str(block_unroll)+'\n\n'
											source_string=source_string+'#define DCT_UNROLL '+str(dct_unroll)+'\n\n'
											source_string=source_string+'#define CL_FILE_NAME '+'"'+cl_file_name+'"' +'\n\n'

											source_string=source_string+'#define print_rsl printf("dse result: %d, %d, %d, %d, %d, %d, %d, %d, %d, %f\\n",'+str(blockdim_x)+','+str(blockdim_y)+','+ str(simd_type)+','+str(simd_loc)+','+ str(block_size_f)+','+ str(block_unroll)+','+ str(dct_unroll)+','+str(simd_wi)+','+str(comp_u)+', ELAPSED_TIME_MS(1, 0)/NUM_ITER)\n\n'


											tmp_string=input_base_file.readline()
                                
                                							while (tmp_string!=''):
                                    								source_string=source_string+tmp_string
                                    								tmp_string=input_base_file.readline()

                                							input_base_file.close()
                                							output_cl_file.write(source_string)
                                							output_cl_file.close()

			
