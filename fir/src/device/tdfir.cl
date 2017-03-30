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
 * Filename: tdfir.cl
 * Version: 1.0
 * Description: FIR filter OpenCL benchmark.
 * Author (modifications): Quentin Gautier
 * Note: This work is based on the Altera OpenCL FIR filter example.
 */

#include "parameters.h"



/****************************************************************************** 

This kernel implements the complex FIR filter.

 ******************************************************************************/

//----------------------------------------------------------------
#ifdef ALTERA_CL

#if TOTAL_WORK_ITEMS == 1
__attribute__((task))
#else
__attribute__ ((reqd_work_group_size(KNOB_NUM_WORK_ITEMS,1,1)))
__attribute__ ((num_simd_work_items(KNOB_SIMD)))
__attribute__ ((num_compute_units(KNOB_COMPUTE_UNITS)))

#endif // TOTAL_WORK_ITEMS
#endif // ALTERA_CL
//----------------------------------------------------------------
__kernel void tdfir (
		__global float *restrict dataPtr, __global float *restrict filterPtr,
		__global float *restrict resultPtr, const int totalInputLength, const int numIterations,
		const int paddedSingleInputLength, const int totalFiltersLength
)
//----------------------------------------------------------------
{
	int tid = get_global_id(0);
	
	// Sliding windows
	float ai_a0[FILTER_LENGTH + KNOB_NUM_PARALLEL-1];
	float ai_b0[FILTER_LENGTH + KNOB_NUM_PARALLEL-1];

	// Filter coefficients
	float coef_real[FILTER_LENGTH];
	float coef_imag[FILTER_LENGTH];

	int ilen, k; 

	// Initialize sliding windows
#pragma unroll
	for(ilen = 0; ilen < FILTER_LENGTH + KNOB_NUM_PARALLEL-1; ilen++)
	{
		ai_a0[ilen] = 0.0f;
		ai_b0[ilen] = 0.0f;
	}

	uchar  load_filter = 1;
	ushort load_filter_index = tid * totalFiltersLength;
	uchar  num_coefs_loaded = 0;
	ushort ifilter = 0;

	
	// *********
	// Main loop
	// *********
#pragma unroll KNOB_UNROLL_TOTAL
	for(ilen = 0; ilen < numIterations; ilen++)
	{
		int currentIdx = 2*tid*totalInputLength + 2*KNOB_NUM_PARALLEL*ilen;
		
		// Local filter results
		float firReal[KNOB_NUM_PARALLEL] = {0.0f};
		float firImag[KNOB_NUM_PARALLEL] = {0.0f};

		// Shift sliding windows
#pragma unroll KNOB_UNROLL_FILTER_1
		for (k=0; k < FILTER_LENGTH-1; k++)
		{
			ai_a0[k] = ai_a0[k+KNOB_NUM_PARALLEL];
			ai_b0[k] = ai_b0[k+KNOB_NUM_PARALLEL];
		}

		// Shift in complex data point(s) to process
#pragma unroll KNOB_UNROLL_PARALLEL_1
		for(k = 0; k < KNOB_NUM_PARALLEL; k++)
		{
			int dataIdx = currentIdx + 2*k;
			ai_a0[k + FILTER_LENGTH-1] = dataPtr[dataIdx]; 
			ai_b0[k + FILTER_LENGTH-1] = dataPtr[dataIdx + 1];
		}

		// Also shift in the filter coefficients for every set of data to process
		//
		// Shift the cofficients in 8 complex points every clock cycle
		// It will take 16 clock cycles to shift all 128 coefficients in.
		// Thus, we need to pad the incoming data with 16 complex points of 0
		// at the beginning of every new dataset to ensure data is aligned.
		//
		// Note: We parameterize the number cited above.
		//
		if (load_filter)
		{
			
			// Shift coefficients
#pragma unroll KNOB_UNROLL_FILTER_2
			for (k=0; k < FILTER_LENGTH-KNOB_COEF_SHIFT; k+=KNOB_COEF_SHIFT)
			{
				
#pragma unroll KNOB_UNROLL_COEF_SHIFT_1
				for(int i = 0; i < KNOB_COEF_SHIFT; i++)
				{
					coef_real[k+i] = coef_real[k+i+KNOB_COEF_SHIFT];
					coef_imag[k+i] = coef_imag[k+i+KNOB_COEF_SHIFT];
				}
			}
			
			// Load new coefficients
#pragma unroll KNOB_UNROLL_COEF_SHIFT_2
			for(int i = 0; i < KNOB_COEF_SHIFT; i++)
			{
				coef_real[FILTER_LENGTH-(KNOB_COEF_SHIFT-i)] = filterPtr[2*load_filter_index+2*i];
				coef_imag[FILTER_LENGTH-(KNOB_COEF_SHIFT-i)] = filterPtr[2*load_filter_index+2*i+1];
			}
			
			load_filter_index += KNOB_COEF_SHIFT;

			if (++num_coefs_loaded == NUM_COEF_LOADS) { load_filter = 0; num_coefs_loaded = 0; }
		}

		
		// This is the core computation of the FIR filter
#pragma unroll KNOB_UNROLL_FILTER_3
		for (k=FILTER_LENGTH-1; k >=0; k--)
		{	
#pragma unroll KNOB_UNROLL_PARALLEL_2
			for(int i = 0; i < KNOB_NUM_PARALLEL; i++)
			{
				firReal[i] += ai_a0[k+i] * coef_real[FILTER_LENGTH-1-k]  - ai_b0[k+i] * coef_imag[FILTER_LENGTH-1-k];
				firImag[i] += ai_a0[k+i] * coef_imag[FILTER_LENGTH-1-k]  + ai_b0[k+i] * coef_real[FILTER_LENGTH-1-k];
			}
		}

		// Writing back the computational result
#pragma unroll KNOB_UNROLL_PARALLEL_3
		for(int i = 0; i < KNOB_NUM_PARALLEL; i++)
		{
			int resultIdx = currentIdx + 2*i;
			
			resultPtr[resultIdx]   = firReal[i];
			resultPtr[resultIdx+1] = firImag[i];
		}

		// The ifilter variable is a counter that counts up to the number of data inputs
		// per filter to process.  When it reaches paddedSingleInputLength, we will know
		// that it is time to load in new filter coefficients for the next batch of data,
		// and reset the counter.
		if (ifilter == paddedSingleInputLength) 
		{
			load_filter = 1;
		}
		if (ifilter == paddedSingleInputLength)
		{ 
			ifilter = 0;
		} 
		else
			ifilter += KNOB_NUM_PARALLEL;
	}
}


// Copyright (C) 2013-2014 Altera Corporation, San Jose, California, USA. All rights reserved.
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

