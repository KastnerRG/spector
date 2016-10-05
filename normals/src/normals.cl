#include "knobs.h"

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
 * Filename: normals.cl
 * Version: 1.0
 * Description: Normal estimation OpenCL benchmark.
 * Author: Quentin Gautier
 * Note: This work is partially based on kinfu from the Point Cloud Library.
 */



// -------------------------------------------------------
inline float3 normalized(const float3 v)
// -------------------------------------------------------
{
	return v * rsqrt(dot(v, v));
}





// ***********************************************
// Design number 1
// ***********************************************


#if KNOB_DESIGN_TYPE==0


// -------------------------------------------------------
/**
 * Compute a normal map based on the vertex map.
 * @param vmap[in]  The input vertex map in interleaved format (X,Y,Z). If there is no vertex information for a value, X should be NaN.
 * @param nmap[out] The output normal map in interleaved format (X,Y,Z). If no normal could be computed at a point, X is set to NaN.
 * @param cols      Width of the maps in number of 3D points.
 * @param rows      Height of the maps in number of 3D points.
 **/
// -------------------------------------------------------
#ifdef ALTERA_CL
__attribute__ ((reqd_work_group_size(KNOB_WORK_ITEMS,1,1)))
__attribute__ ((num_simd_work_items(1)))
__attribute__ ((num_compute_units(KNOB_COMPUTE_UNITS)))
#endif

__kernel void computeNmap_v3(
		__global float* restrict  vmap,
		__global float* restrict  nmap,
		int cols, int rows
)
// -------------------------------------------------------
{
	const int tid           = get_global_id(0);
	const int num_blocks    = TOTAL_WORK_ITEMS;
	
	// Divide rows into work-items
	const int rows_in_block = rows / num_blocks;
	const int block_size    = rows_in_block * cols * 3;
	const int last_row      = 3 * cols * (rows-1);

	const int start         = block_size * tid;
	const int end           = min(block_size * (tid+1), last_row);


	// Sliding window
	float data0[(1+KNOB_WINDOW_SIZE_X)*3];
	float data1[KNOB_WINDOW_SIZE_X*3];
	
	// Initialize sliding window
#pragma unroll
	for(int i = 0; i < (1+KNOB_WINDOW_SIZE_X)*3; i++)
	{
		data0[i] = vmap[start + i];
	}
#pragma unroll
	for(int i = 0; i < KNOB_WINDOW_SIZE_X*3; i++)
	{
		data1[i] = vmap[start + 3*cols + i];
	}



	// Loop over each point to process
#pragma unroll KNOB_UNROLL
	for(int i = start, j = 0; i < end; i+=KNOB_WINDOW_SIZE_X*3)
	{

		// Loop over each point inside the sliding window
#pragma unroll KNOB_UNROLL_INNER
		for(int x = 0; x < KNOB_WINDOW_SIZE_X; x++, j++)
		{
			const int index00 = (x + 0) * 3;
			const int index01 = (x + 1) * 3;
			const int index10 = (x + 0) * 3;


			// If last columnn, fill with NaN
			if (j == cols - 1)
			{
				j = -1;
				nmap[i + x*3] = NAN;
			}

			// Calculate normal
			else
			{
				float3 v00, v01, v10;
				v00.x = data0[index00];
				v01.x = data0[index01];
				v10.x = data1[index10];

				if (!isnan (v00.x) && !isnan (v01.x) && !isnan (v10.x))
				{
					v00.y = data0[index00 + 1];
					v01.y = data0[index01 + 1];
					v10.y = data1[index10 + 1];

					v00.z = data0[index00 + 2];
					v01.z = data0[index01 + 2];
					v10.z = data1[index10 + 2];


					float3 r = normalized (cross (v01 - v00, v10 - v00));

					nmap[i + 0 + x*3] = r.x;
					nmap[i + 1 + x*3] = r.y;
					nmap[i + 2 + x*3] = r.z;
				}
				else
				{
					nmap[i + x*3] = NAN;
				}
			}
		}
	





		// Shift window
		//
		data0[0] = data0[KNOB_WINDOW_SIZE_X * 3 + 0];
		data0[1] = data0[KNOB_WINDOW_SIZE_X * 3 + 1];
		data0[2] = data0[KNOB_WINDOW_SIZE_X * 3 + 2];

#pragma unroll
		for(int x = 0; x < KNOB_WINDOW_SIZE_X; x++)
		{
			const int index = (x+1) * 3;
			data0[index + 0] = vmap[i+KNOB_WINDOW_SIZE_X*3 + index + 0];
			data0[index + 1] = vmap[i+KNOB_WINDOW_SIZE_X*3 + index + 1];
			data0[index + 2] = vmap[i+KNOB_WINDOW_SIZE_X*3 + index + 2];
		}

#pragma unroll
		for(int x = 0; x < KNOB_WINDOW_SIZE_X; x++)
		{
			// Can blow up if KNOB_WINDOW_SIZE_X does not divide cols
			data1[x*3 + 0] = vmap[3*cols + i+KNOB_WINDOW_SIZE_X*3 + x*3 + 0];
			data1[x*3 + 1] = vmap[3*cols + i+KNOB_WINDOW_SIZE_X*3 + x*3 + 1];
			data1[x*3 + 2] = vmap[3*cols + i+KNOB_WINDOW_SIZE_X*3 + x*3 + 2];
		}
	}


	// If last row, fill with NaN
	if(tid == num_blocks-1)
	{
		for(int i = last_row; i < 3*rows*cols; i+=3)
		{
			nmap[i] = NAN;
		}
	}
	
}





// ***********************************************
// Design number 2
// ***********************************************
// (very similar, but uses the work-items to process each elements of the window)


#else


#if KNOB_WORK_ITEMS == KNOB_WINDOW_SIZE_X
#else
#error KNOB_WORK_ITEMS and KNOB_WINDOW_SIZE_X must be equal!
#endif


// -------------------------------------------------------
/**
 * Compute a normal map based on the vertex map.
 * @param vmap[in]  The input vertex map in interleaved format (X,Y,Z). If there is no vertex information for a value, X should be NaN.
 * @param nmap[out] The output normal map in interleaved format (X,Y,Z). If no normal could be computed at a point, X is set to NaN.
 * @param cols      Width of the maps in number of 3D points.
 * @param rows      Height of the maps in number of 3D points.
 **/
// -------------------------------------------------------
#ifdef ALTERA_CL
__attribute__ ((reqd_work_group_size(KNOB_WORK_ITEMS,1,1)))
__attribute__ ((num_simd_work_items(1)))
__attribute__ ((num_compute_units(KNOB_COMPUTE_UNITS)))
#endif

__kernel void computeNmap_v3(
		__global float* restrict  vmap,
		__global float* restrict  nmap,
		int cols, int rows
)
// -------------------------------------------------------
{
	const int lid           = get_local_id(0);  // local ID
	const int bid           = get_group_id(0);  // block ID
	const int num_blocks    = KNOB_WORK_GROUPS;
	 
	// Divide rows into work-items
	const int rows_in_block = rows / num_blocks;
	const int block_size    = rows_in_block * cols * 3;
	const int last_row      = 3 * cols * (rows-1);

	const int start         = block_size * bid;
	const int end           = min(block_size * (bid+1), last_row);


	// Sliding window
	local float data0[(1+KNOB_WINDOW_SIZE_X)*3];
	local float data1[KNOB_WINDOW_SIZE_X*3];
	
	// Initialize sliding window
	if(lid == 0)
	{
#pragma unroll
		for(int i = 0; i < 3; i++)
		{
			data0[i] = vmap[start + i];
		}
	}
#pragma unroll
	for(int i = 0; i < 3; i++)
	{
		data0[i + (lid+1)*3] = vmap[start + i + (lid+1)*3];
	}
#pragma unroll
	for(int i = 0; i < 3; i++)
	{
		data1[i + lid*3] = vmap[start + 3*cols + i + lid*3];
	}



	const int x = lid;
	
	local int j;

	j = 0;
	
	// Loop over each point to process
#pragma unroll KNOB_UNROLL
	for(int i = start; i < end; i+=KNOB_WINDOW_SIZE_X*3)
	{

		barrier(CLK_LOCAL_MEM_FENCE);

		{
			const int index00 = (x + 0) * 3;
			const int index01 = (x + 1) * 3;
			const int index10 = (x + 0) * 3;


			// If last columnn, fill with NaN
			if (j+x == cols - 1)
			{
				j = -1-x;
				nmap[i + x*3] = NAN;
			}

			// Calculate normal
			else
			{
				float3 v00, v01, v10;
				v00.x = data0[index00];
				v01.x = data0[index01];
				v10.x = data1[index10];

				if (!isnan (v00.x) && !isnan (v01.x) && !isnan (v10.x))
				{
					v00.y = data0[index00 + 1];
					v01.y = data0[index01 + 1];
					v10.y = data1[index10 + 1];

					v00.z = data0[index00 + 2];
					v01.z = data0[index01 + 2];
					v10.z = data1[index10 + 2];


					float3 r = normalized (cross (v01 - v00, v10 - v00));

					nmap[i + 0 + x*3] = r.x;
					nmap[i + 1 + x*3] = r.y;
					nmap[i + 2 + x*3] = r.z;
				}
				else
				{
					nmap[i + x*3] = NAN;
				}
			}
		}
	

		barrier(CLK_LOCAL_MEM_FENCE);




		// Shift window
		//
		if(lid == 0)
		{
			data0[0] = data0[KNOB_WINDOW_SIZE_X * 3 + 0];
			data0[1] = data0[KNOB_WINDOW_SIZE_X * 3 + 1];
			data0[2] = data0[KNOB_WINDOW_SIZE_X * 3 + 2];
		}

		{
			const int index = (x+1) * 3;
			data0[index + 0] = vmap[i+KNOB_WINDOW_SIZE_X*3 + index + 0];
			data0[index + 1] = vmap[i+KNOB_WINDOW_SIZE_X*3 + index + 1];
			data0[index + 2] = vmap[i+KNOB_WINDOW_SIZE_X*3 + index + 2];
		}

		{
			data1[x*3 + 0] = vmap[3*cols + i+KNOB_WINDOW_SIZE_X*3 + x*3 + 0];
			data1[x*3 + 1] = vmap[3*cols + i+KNOB_WINDOW_SIZE_X*3 + x*3 + 1];
			data1[x*3 + 2] = vmap[3*cols + i+KNOB_WINDOW_SIZE_X*3 + x*3 + 2];
		}

		if(lid == 0){ j += KNOB_WINDOW_SIZE_X; }
	}


	// If last row, fill with NaN
	if(bid == num_blocks-1 && lid == 0)
	{
		for(int i = last_row; i < 3*rows*cols; i+=3)
		{
			nmap[i] = NAN;
		}
	}
	
}


#endif // KNOB_DESIGN_TYPE


/*
 * Software License Agreement (BSD License)
 *
 *  Point Cloud Library (PCL) - www.pointclouds.org
 *  Copyright (c) 2011, Willow Garage, Inc.
 *
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions
 *  are met:
 *
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other materials provided
 *     with the distribution.
 *   * Neither the name of Willow Garage, Inc. nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 *  FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 *  COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 *  INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 *  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 *  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 *  ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 */




