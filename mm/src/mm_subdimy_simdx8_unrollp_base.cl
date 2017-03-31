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
 * Filename: mm_subdimy_simdx8_unrollp_base.cl
 * Version: 1.0
 * Description: Matrix multiplication OpenCL benchmark.
 * Author: Pingfan Meng
 */


__attribute__((reqd_work_group_size(BLOCKDIM,BLOCKDIM,1)))
__kernel void matrixMult( // Input and output matrices
                 __global float * restrict C,
                 __global float * restrict A,
                 __global float * restrict B, 
                 // Widths of matrices.
                 int M)
{
        // Local storage for a block of input matrices A and B
        __local float A_local[BLOCKDIM*SUBDIM_Y][BLOCKDIM];
        __local float8 B_local[BLOCKDIM][BLOCKDIM];

        // Block index
        __private int block_x = get_group_id(0);
        __private int block_y = get_group_id(1);

        // Local ID index (offset within a block)
        __private int local_x = get_local_id(0);
        __private int local_y = get_local_id(1);

    
        // Compute loop bounds
        __private int a_start = M * BLOCKDIM * SUBDIM_Y * block_y;
        __private int a_end   = a_start + M;
        __private int b_start = BLOCKDIM  * SIMD_X *block_x;
             

    __private float8 running_sum[SUBDIM_Y];

    __private int k;

    __private int p;


    #pragma unroll UNROLL_F
    for (p=0;p<SUBDIM_Y;p++)
    {
        running_sum[p]=(float8)(0.0f,0.0f,0.0f,0.0f,
                    0.0f,0.0f,0.0f,0.0f);
    }

    // Compute the matrix multiplication result for this output element. Each
    // loop iteration processes one block of the matrix.
    for (int a = a_start, b = b_start; a < a_end; a += BLOCKDIM, b += (BLOCKDIM * M))
    {
        
	#pragma unroll UNROLL_F
    	for (p=0;p<SUBDIM_Y;p++)
        {
        	A_local[p*BLOCKDIM+local_y][local_x] = A[a + M * (local_y+p*BLOCKDIM) + local_x];
	}

        
        B_local[local_x][local_y].s0 = B[b + M * local_y + SIMD_X*local_x];
	B_local[local_x][local_y].s1 = B[b + M * local_y + SIMD_X*local_x+1];
	B_local[local_x][local_y].s2 = B[b + M * local_y + SIMD_X*local_x+2];
	B_local[local_x][local_y].s3 = B[b + M * local_y + SIMD_X*local_x+3];
	B_local[local_x][local_y].s4 = B[b + M * local_y + SIMD_X*local_x+4];
	B_local[local_x][local_y].s5 = B[b + M * local_y + SIMD_X*local_x+5];
	B_local[local_x][local_y].s6 = B[b + M * local_y + SIMD_X*local_x+6];
	B_local[local_x][local_y].s7 = B[b + M * local_y + SIMD_X*local_x+7];
        
    
        // Wait for the entire block to be loaded.
        barrier(CLK_LOCAL_MEM_FENCE);

        
        for (k = 0; k < BLOCKDIM; k++)
        {
            #pragma unroll UNROLL_F
            for (p=0;p<SUBDIM_Y;p++)
            {
                running_sum[p].s0 += A_local[local_y*SUBDIM_Y+p][k] * B_local[local_x][k].s0;
                running_sum[p].s1 += A_local[local_y*SUBDIM_Y+p][k] * B_local[local_x][k].s1;
                running_sum[p].s2 += A_local[local_y*SUBDIM_Y+p][k] * B_local[local_x][k].s2;
                running_sum[p].s3 += A_local[local_y*SUBDIM_Y+p][k] * B_local[local_x][k].s3;
                running_sum[p].s4 += A_local[local_y*SUBDIM_Y+p][k] * B_local[local_x][k].s4;
                running_sum[p].s5 += A_local[local_y*SUBDIM_Y+p][k] * B_local[local_x][k].s5;
                running_sum[p].s6 += A_local[local_y*SUBDIM_Y+p][k] * B_local[local_x][k].s6;
                running_sum[p].s7 += A_local[local_y*SUBDIM_Y+p][k] * B_local[local_x][k].s7;
            }
        }

        // Wait for the block to be fully consumed before loading the next
        // block.
        barrier(CLK_LOCAL_MEM_FENCE);
    }//end of for
    
    
        // Store result in matrix C

    #pragma unroll UNROLL_F
    for (p=0;p<SUBDIM_Y;p++)
    {
        C[(get_global_id(1) *SUBDIM_Y+p) * M + get_global_id(0)*SIMD_X] = running_sum[p].s0;
        C[(get_global_id(1) *SUBDIM_Y+p) * M + get_global_id(0)*SIMD_X+1] = running_sum[p].s1;
        C[(get_global_id(1) *SUBDIM_Y+p) * M + get_global_id(0)*SIMD_X+2] = running_sum[p].s2;
        C[(get_global_id(1) *SUBDIM_Y+p) * M + get_global_id(0)*SIMD_X+3] = running_sum[p].s3;
        C[(get_global_id(1) *SUBDIM_Y+p) * M + get_global_id(0)*SIMD_X+4] = running_sum[p].s4;
        C[(get_global_id(1) *SUBDIM_Y+p) * M + get_global_id(0)*SIMD_X+5] = running_sum[p].s5;
        C[(get_global_id(1) *SUBDIM_Y+p) * M + get_global_id(0)*SIMD_X+6] = running_sum[p].s6;
        C[(get_global_id(1) *SUBDIM_Y+p) * M + get_global_id(0)*SIMD_X+7] = running_sum[p].s7;
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
