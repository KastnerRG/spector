// ----------------------------------------------------------------------
// Copyright (c) 2016, The Regents of the University of California All
// rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// 
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
// 
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
// 
//     * Neither the name of The Regents of the University of California
//       nor the names of its contributors may be used to endorse or
//       promote products derived from this software without specific
//       prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL REGENTS OF THE
// UNIVERSITY OF CALIFORNIA BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
// TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
// USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
// DAMAGE.
// ----------------------------------------------------------------------
/*
 * Filename: sobel_subdimy_simdx1_base.cl
 * Version: 1.0
 * Description: Sobel filter OpenCL benchmark.
 * Author: Pingfan Meng
 */


__attribute__((reqd_work_group_size(BLOCKDIM_X,BLOCKDIM_Y,1)))
__kernel void sobel_filter(__global unsigned int * restrict image_in, __global unsigned int * restrict image_out)
{
   	
	__private int col_id=get_global_id(0)*SIMD_X;
	__private int row_id=get_global_id(1)*SUBDIM_Y;
	
	__private int loc_col_id=get_local_id(0)*SIMD_X;
	__private int loc_row_id=get_local_id(1)*SUBDIM_Y;
	
	__local int loc_buffer[BLOCKDIM_Y*SUBDIM_Y][BLOCKDIM_X*SIMD_X];

	__private int wx[3][3]={{-1,-2,-1},{0,0,0},{1,2,1}};
	__private int wy[3][3]={{-1,0,1},{-2,0,2},{-1,0,1}};

	__private int image_reg[3][2+SIMD_X];

	__private int resultx,resulty;

	__private int i,j,p;

	__private uint clamped;
	__private int temp;

	__private uint pixel,b,g,r,luma;




	#pragma unroll
	for (p=0;p<SUBDIM_Y;p++)
	{
		loc_buffer[loc_row_id+p][loc_col_id]=image_in[(row_id+p)*WIDTH+col_id];
	}
	

	barrier(CLK_LOCAL_MEM_FENCE);


	//load in reg00
	image_reg[0][0] = (row_id==0||col_id==0)? 0 : ((loc_row_id==0||loc_col_id==0)? image_in[(row_id-1)*WIDTH+col_id-1]:loc_buffer[loc_row_id-1][loc_col_id-1]);
	
	//load in reg01
	image_reg[0][1] = (row_id==0)? 0 : ((loc_row_id==0)? image_in[(row_id-1)*WIDTH+col_id]:loc_buffer[loc_row_id-1][loc_col_id]);
	
	//load in reg02	
	image_reg[0][2] = (row_id==0||col_id==WIDTH-1)? 0 : ((loc_row_id==0||loc_col_id==BLOCKDIM_X-1)? image_in[(row_id-1)*WIDTH+col_id+1]:loc_buffer[loc_row_id-1][loc_col_id+1]);

	//load in reg10	
	image_reg[1][0] = (col_id==0)? 0 : ((loc_col_id==0)? image_in[row_id*WIDTH+col_id-1]:loc_buffer[loc_row_id][loc_col_id-1]);

	
	//load in reg11
	image_reg[1][1] = loc_buffer[loc_row_id][loc_col_id];


	//load in reg12
	image_reg[1][2] = (col_id==WIDTH-1)? 0 : ((loc_col_id==BLOCKDIM_X-1)? image_in[row_id*WIDTH+col_id+1]:loc_buffer[loc_row_id][loc_col_id+1]) ;

	//load in reg20
	image_reg[2][0] = (row_id==HEIGHT-1||col_id==0)? 0 : ((loc_row_id==BLOCKDIM_Y*SUBDIM_Y-1||loc_col_id==0)? image_in[(row_id+1)*WIDTH+col_id-1]:loc_buffer[loc_row_id+1][loc_col_id-1]);
	
	//load in reg21
	image_reg[2][1] = (row_id==HEIGHT-1)? 0 : ((loc_row_id==BLOCKDIM_Y*SUBDIM_Y-1)? image_in[(row_id+1)*WIDTH+col_id]:loc_buffer[loc_row_id+1][loc_col_id]);
		
	//load in reg22
	image_reg[2][2] = (row_id==HEIGHT-1||col_id==WIDTH-1)? 0 : ((loc_row_id==BLOCKDIM_Y*SUBDIM_Y-1||loc_col_id==BLOCKDIM_X-1)?image_in[(row_id+1)*WIDTH+col_id+1]:loc_buffer[loc_row_id+1][loc_col_id+1]);


		for (p=0;p<SUBDIM_Y;p++)
		{
			//filter:
			resultx=0;
			resulty=0;
			#pragma unroll 3
			for (i=0;i<3;i++)
			{
				#pragma unroll 3
				for (j=0;j<3;j++)
				{
					pixel = image_reg[i][j];
					b = pixel & 0xff;
					g = (pixel >> 8) & 0xff;
					r = (pixel >> 16) & 0xff;

					// RGB -> Luma conversion approximation
					// Avoiding floating point math operators greatly reduces
					// resource usage.
					luma = r * 66 + g * 129 + b * 25;
					luma = (luma + 128) >> 8;
					luma =luma + 16;

					resultx=resultx+wx[i][j]*luma;
					resulty=resulty+wy[i][j]*luma;

				}
			}
	
			temp=abs(resultx)+abs(resulty);
            
            		clamped=temp>32?1:0;
			
			image_out[(row_id+p)*WIDTH+col_id]=clamped;
			
			if (p+1<SUBDIM_Y)
			{
				//shift the reg row
				image_reg[0][0]=image_reg[1][0];
				image_reg[0][1]=image_reg[1][1];
				image_reg[0][2]=image_reg[1][2];

				image_reg[1][0]=image_reg[2][0];
				image_reg[1][1]=image_reg[2][1];
				image_reg[1][2]=image_reg[2][2];

				//load in reg20
				image_reg[2][0] = (row_id+p+1==HEIGHT-1||col_id==0)? 0 : ((loc_row_id+p+1==BLOCKDIM_Y*SUBDIM_Y-1||loc_col_id==0)? image_in[(row_id+p+1+1)*WIDTH+col_id-1]:loc_buffer[loc_row_id+p+1+1][loc_col_id-1]);
			

				//load in reg21
				image_reg[2][1] = (row_id+p+1==HEIGHT-1)? 0 : ((loc_row_id+p+1==BLOCKDIM_Y*SUBDIM_Y-1)? image_in[(row_id+p+1+1)*WIDTH+col_id]:loc_buffer[loc_row_id+p+1+1][loc_col_id]);
				

				//load in reg22
				image_reg[2][2] = (row_id+p+1==HEIGHT-1||col_id==WIDTH-1)? 0 : ((loc_row_id+p+1==BLOCKDIM_Y*SUBDIM_Y-1||loc_col_id==BLOCKDIM_X-1)?image_in[(row_id+p+1+1)*WIDTH+col_id+1]:loc_buffer[loc_row_id+p+1+1][loc_col_id+1]);

			}	

		}
}


// Copyright (C) 2013-2015 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.
