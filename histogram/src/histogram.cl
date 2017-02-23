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
 * Filename: histogram.cl
 * Version: 1.0
 * Description: Histogram calculation OpenCL benchmark.
 * Author: Quentin Gautier
 */

#include "params.h"



// -------------------------------------------------------
/**
 * Calculate a histogram of the input 8-bits values.
 * @param[in]  data       The input data to count.
 * @param[out] histogram  The resulting histogram. Each bin contains the number of values equals to the bin index in the input data.
 * @param[in]  numData    The number of values to count in the input data.
 **/
// -------------------------------------------------------
#ifdef ALTERA_CL
__attribute__ ((reqd_work_group_size(KNOB_NUM_WORK_ITEMS,1,1)))
__attribute__ ((num_simd_work_items(KNOB_SIMD)))
__attribute__ ((num_compute_units(KNOB_COMPUTE_UNITS)))
#endif

__kernel void calculateHistogram(
		__global const unsigned char*  restrict  data,
		__global       unsigned int*   restrict  histogram,
		int numData
)
// -------------------------------------------------------
{

	// If using multiple work-items (and work-groups), divide the input data into the work-items.
#if TOTAL_WORK_ITEMS > 1
	const int tid = get_global_id(0);
#else
	const int tid = 0;
#endif
	const int startIdx = tid * numData;
	

	// Local buffer(s)
	//
	unsigned int hist1[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 2
	unsigned int hist2[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 3
	unsigned int hist3[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 4
	unsigned int hist4[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 5
	unsigned int hist5[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 6
	unsigned int hist6[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 7
	unsigned int hist7[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 8
	unsigned int hist8[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 9
	unsigned int hist9[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 10
	unsigned int hist10[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 11
	unsigned int hist11[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 12
	unsigned int hist12[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 13
	unsigned int hist13[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 14
	unsigned int hist14[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 15
	unsigned int hist15[KNOB_HIST_SIZE];
	
#if KNOB_NUM_HIST >= 16
	unsigned int hist16[KNOB_HIST_SIZE];

#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
	
	// Initialize the local buffer(s)
	for(int i = 0; i < KNOB_HIST_SIZE; i++)
	{
		hist1[i] = 0;
		
#if KNOB_NUM_HIST >= 2
		hist2[i] = 0;
		
#if KNOB_NUM_HIST >= 3
		hist3[i] = 0;
		
#if KNOB_NUM_HIST >= 4
		hist4[i] = 0;
		
#if KNOB_NUM_HIST >= 5
		hist5[i] = 0;
		
#if KNOB_NUM_HIST >= 6
		hist6[i] = 0;
		
#if KNOB_NUM_HIST >= 7
		hist7[i] = 0;
		
#if KNOB_NUM_HIST >= 8
		hist8[i] = 0;
		
#if KNOB_NUM_HIST >= 9
		hist9[i] = 0;
		
#if KNOB_NUM_HIST >= 10
		hist10[i] = 0;
		
#if KNOB_NUM_HIST >= 11
		hist11[i] = 0;
		
#if KNOB_NUM_HIST >= 12
		hist12[i] = 0;
		
#if KNOB_NUM_HIST >= 13
		hist13[i] = 0;
		
#if KNOB_NUM_HIST >= 14
		hist14[i] = 0;
		
#if KNOB_NUM_HIST >= 15
		hist15[i] = 0;
		
#if KNOB_NUM_HIST >= 16
		hist16[i] = 0;
		
#endif
#endif
#endif
#endif
#endif
#endif
#endif		
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
	}

	// Count the input data into the local buffer(s)
#pragma unroll KNOB_UNROLL_FACTOR
	for(int i = 0; i < numData; i+=KNOB_NUM_HIST)
	{
		int index = i + startIdx;
		
		hist1[data[index]]++;
		
#if KNOB_NUM_HIST >= 2
		if(i+1 < numData){ hist2[data[index+1]]++; }
		
#if KNOB_NUM_HIST >= 3
		if(i+2 < numData){ hist3[data[index+2]]++; }
		
#if KNOB_NUM_HIST >= 4
		if(i+3 < numData){ hist4[data[index+3]]++; }
		
#if KNOB_NUM_HIST >= 5
		if(i+4 < numData){ hist5[data[index+4]]++; }
		
#if KNOB_NUM_HIST >= 6
		if(i+5 < numData){ hist6[data[index+5]]++; }
		
#if KNOB_NUM_HIST >= 7
		if(i+6 < numData){ hist7[data[index+6]]++; }
		
#if KNOB_NUM_HIST >= 8
		if(i+7 < numData){ hist8[data[index+7]]++; }
		
#if KNOB_NUM_HIST >= 9
		if(i+8 < numData){ hist9[data[index+8]]++; }
		
#if KNOB_NUM_HIST >= 10
		if(i+9 < numData){ hist10[data[index+9]]++; }
		
#if KNOB_NUM_HIST >= 11
		if(i+10 < numData){ hist11[data[index+10]]++; }
		
#if KNOB_NUM_HIST >= 12
		if(i+11 < numData){ hist12[data[index+11]]++; }
		
#if KNOB_NUM_HIST >= 13
		if(i+12 < numData){ hist13[data[index+12]]++; }
		
#if KNOB_NUM_HIST >= 14
		if(i+13 < numData){ hist14[data[index+13]]++; }
		
#if KNOB_NUM_HIST >= 15
		if(i+14 < numData){ hist15[data[index+14]]++; }
		
#if KNOB_NUM_HIST >= 16
		if(i+15 < numData){ hist16[data[index+15]]++; }
		
#endif
#endif
#endif
#endif
#endif
#endif
#endif		
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif

	}
	

	

	// Shared memory buffer to accumulate the results of different work-items.
#if KNOB_ACCUM_SMEM == 1
	local unsigned int hist_smem[256 * KNOB_NUM_WORK_ITEMS];
#endif
	
	

	// Accumulate the local buffers (if using several).
	for(int i = 0; i < 256; i++)
	{
		
#if KNOB_ACCUM_SMEM == 0
		histogram[tid * 256 + i] =
#else
		hist_smem[tid * 256 + i] =
#endif
				
				hist1[i] +

#if KNOB_NUM_HIST >= 2
				hist2[i] +

#if KNOB_NUM_HIST >= 3
				hist3[i] +

#if KNOB_NUM_HIST >= 4
				hist4[i] +

#if KNOB_NUM_HIST >= 5
				hist5[i] +

#if KNOB_NUM_HIST >= 6
				hist6[i] +

#if KNOB_NUM_HIST >= 7
				hist7[i] +

#if KNOB_NUM_HIST >= 8
				hist8[i] +

#if KNOB_NUM_HIST >= 9
				hist9[i] +

#if KNOB_NUM_HIST >= 10
				hist10[i] +

#if KNOB_NUM_HIST >= 11
				hist11[i] +

#if KNOB_NUM_HIST >= 12
				hist12[i] +

#if KNOB_NUM_HIST >= 13
				hist13[i] +

#if KNOB_NUM_HIST >= 14
				hist14[i] +

#if KNOB_NUM_HIST >= 15
				hist15[i] +

#if KNOB_NUM_HIST >= 16
				hist16[i] +
				
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
#endif
				0;
	}
	
	
	// Accumulate the results of each work-item in shared memory (if using only one work-group)
#if KNOB_ACCUM_SMEM == 1
	barrier(CLK_LOCAL_MEM_FENCE);
	
	if(tid == 0)
	{
		for(int i = 1; i < KNOB_NUM_WORK_ITEMS; i++)
		{
			for(int j = 0; j < 256; j++)
			{
				hist_smem[j] += hist_smem[i * 256 + j];
			}
		}
		
		for(int j = 0; j < 256; j++)
		{
			histogram[j] = hist_smem[j];
		}
	}
#endif
	

}


// -------------------------------------------------------
/**
 * Accumulate multiple histograms of length 256.
 * @param[in]  histogram_in   A contiguous list of all the histograms to accumulate. There is TOTAL_WORK_ITEMS number of histograms.
 * @param[out] histogram_out  The resulting 256-bins histogram.
 **/
#if TOTAL_WORK_ITEMS > 1 && KNOB_ACCUM_SMEM == 0
// -------------------------------------------------------
#ifdef ALTERA_CL
__attribute__ ((reqd_work_group_size(1,1,1)))
__attribute__ ((num_simd_work_items(1)))
__attribute__ ((num_compute_units(1)))
#endif

__kernel void accumulateHistograms(
		__global       unsigned int*   restrict  histogram_in,
		__global       unsigned int*   restrict  histogram_out
)
// -------------------------------------------------------
{
	// Local buffer
	unsigned int hist[256];
	
	// Copy the first input histogram into the local buffer.
	for(int j = 0; j < 256; j++)
	{
		hist[j] = histogram_in[j];
	}
	
	// Add each histogram to the local buffer.
	for(int i = 1; i < TOTAL_WORK_ITEMS; i++)
	{
		
		for(int j = 0; j < 256; j++)
		{
			hist[j] += histogram_in[i * 256 + j];
		}
		
	}
	
	// Copy the local buffer to the output.
	for(int j = 0; j < 256; j++)
	{
		histogram_out[j] = hist[j];
	}
}
#endif








