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
 * Filename: mergesort.cl
 * Version: 1.0
 * Description: Merge sort OpenCL benchmark.
 * Author: Quentin Gautier
 */


#include "knobs.h"



// Note: This include may cause issues with some OpenCL drivers (ie. compiles fine, but runtime result is wrong)
// To solve it, you can copy/paste the content of the include here and include directly this .cl file in the main file,
// but you need to uncomment the compile-time check for OpenCL compiler below.





//#if defined(__OPENCL_VERSION__) || defined(ALTERA_CL) // Is this the OpenCL compiler?



// -------------------------------------------------------
/**
 * Sort a buffer of size LOCAL_SORT_SIZE using Merge Sort in local memory.
 *
 * @param  buffer1      the input buffer, may be modified
 * @param  buffer2      the output buffer, must be different from buffer1
 * @param  global_left  the global index on the buffer specifying where to start the sorting
 * @param  tid          the local ID of the work-item
 **/
// -------------------------------------------------------
inline void local_merge_sort(
		global data_t* restrict  buffer1,
		global data_t* restrict  buffer2,
		int global_left, int tid
		)
// -------------------------------------------------------
{
#if KNOB_LOCAL_USE_PTR == 1
	local data_t data1[LOCAL_SORT_SIZE];
	local data_t data2[LOCAL_SORT_SIZE];

	local data_t* localdata1 = data1;
	local data_t* localdata2 = data2;
#else
	local data_t localdata1[LOCAL_SORT_SIZE];
	local data_t localdata2[LOCAL_SORT_SIZE];
#endif

	// Copy the data to local storage
	//
	if(tid == 0)
	{
#pragma unroll KNOB_UNROLL_LOCAL_COPY
		for(int i = 0; i < LOCAL_SORT_SIZE; i++)
		{
			localdata1[i] = buffer1[i+global_left];
		}
	}
#if KNOB_WORK_ITEMS > 1
	barrier(CLK_LOCAL_MEM_FENCE);
#endif


	// Merge chunks of size 1, then 2, 4, 8, etc. until LOCAL_SORT_SIZE
	//
	for(int log_chunk_size = 0; log_chunk_size < KNOB_LOCAL_SORT_LOGSIZE; log_chunk_size++)
	{
		const int chunk_size = (1 << log_chunk_size);

		// Divide chunks among work-items
		//
		const int loopstep  = chunk_size << 1;
		const int loopblock = max(1, (LOCAL_SORT_SIZE >> (log_chunk_size+1)) / KNOB_WORK_ITEMS) << (log_chunk_size+1);
		const int loopstart = tid * loopblock;
		const int loopend   = (tid==KNOB_WORK_ITEMS-1) ? (LOCAL_SORT_SIZE-1) : (min((tid+1) * loopblock, LOCAL_SORT_SIZE-1));	

		// For each chunk (in the current work-item)
		//
		for(int left = loopstart; left < loopend; left += loopstep)
		{
			const int mid   = left + chunk_size - 1;

			const int size_left  = chunk_size;
			const int size_right = chunk_size;


			// Merge from input array to output[left..right]
			//
			int i = 0;    // counter for left elements
			int j = 0;    // counter for right elements
			int k = left; // counter for output

			while (i < size_left && j < size_right)
			{
				data_t left_val  = localdata1[i+left];
				data_t right_val = localdata1[j+mid+1];


				if (left_val <= right_val){ localdata2[k] = left_val; i++; }

				else{ localdata2[k] = right_val; j++; }

				k++;
			}

			while (i < size_left)
			{
				localdata2[k] = localdata1[i+left];
				i++; k++;
			}

			while (j < size_right)
			{
				localdata2[k] = localdata1[j+mid+1];
				j++; k++;
			}

		} // for each chunk


		// Either swap pointers of buffer1/buffer2, or copy the data from buffer2 to buffer1.
		//
#if KNOB_LOCAL_USE_PTR == 1
		// Swap pointers
		local data_t* temp_ptr = localdata1;
		localdata1 = localdata2;
		localdata2 = temp_ptr;
#else

	#if KNOB_WORK_ITEMS > 1
		barrier(CLK_LOCAL_MEM_FENCE);
	#endif
		// Copy data
		if(tid == 0)
		{
			for(int i = 0; i < LOCAL_SORT_SIZE; i++)
			{
				localdata1[i] = localdata2[i];
			}
		}
#endif // if KNOB_LOCAL_USE_PTR

#if KNOB_WORK_ITEMS > 1
		barrier(CLK_LOCAL_MEM_FENCE);
#endif

	}// for chunck size


#if KNOB_LOCAL_USE_PTR == 1
	localdata2 = localdata1;
#endif


	// Copy the data back to global storage
	//
	if(tid == 0)
	{
#pragma unroll KNOB_UNROLL_LOCAL_COPY
		for(int i = 0; i < LOCAL_SORT_SIZE; i++)
		{
			buffer2[i+global_left] = localdata2[i];
		}
	}

#if KNOB_WORK_ITEMS > 1
		barrier(CLK_GLOBAL_MEM_FENCE);
#endif
}



// -------------------------------------------------------
/**
 * Merge two chunks of data in global storage.
 *
 * @param  in_ptr   Input buffer
 * @param  out_ptr  Output buffer
 * @param  left     Index of first element in left chunk
 * @param  mid      Index of last element of left chunk
 * @param  right    Index of last element of right chunk
 **/
// -------------------------------------------------------
inline void global_merge_sort(
		global data_t* restrict  in_ptr,
		global data_t* restrict  out_ptr,
		int left, int mid, int right
		)
// -------------------------------------------------------
{	
	const int size_left  = mid - left + 1;
	const int size_right = right - mid;

	
	// Merge from input array to output[left..right]
	//
	int i = 0;    // counter for left elements
	int j = 0;    // counter for right elements
	int k = left; // counter for output

	while (i < size_left && j < size_right)
	{
		data_t left_val  = in_ptr[i+left];
		data_t right_val = in_ptr[j+mid+1];


		if (left_val <= right_val)
		{
			out_ptr[k] = left_val;
			i++;
		}
		else
		{
			out_ptr[k] = right_val;
			j++;
		}
		k++;
	}

	while (i < size_left)
	{
		out_ptr[k] = in_ptr[i+left];
		i++;
		k++;
	}

	while (j < size_right)
	{
		out_ptr[k] = in_ptr[j+mid+1];
		j++;
		k++;
	}
}





// -------------------------------------------------------
/**
 * OpenCL kernel that applies Merge Sort on the input data.
 * If multiple work-groups are used: Each work-group applies the algorithm on one portion of the buffer, and the kernel must be launched again to do the final merge.
 *
 * @param[in,out]  buffer1           Input buffer, may contain the output.
 * @param[out]     buffer2           Buffer that may contain the output, must be the same size as buffer1.
 * @param[in]      num_data          Number of elements to sort.
 * @param[in]      start_chunk_size  Size of the chunks that are already sorted (the algorithm will start to merge at this size).
 **/
// -------------------------------------------------------
#ifdef ALTERA_CL
	__attribute__ ((reqd_work_group_size(KNOB_WORK_ITEMS,1,1)))
	__attribute__ ((num_simd_work_items(1)))
	__attribute__ ((num_compute_units(KNOB_COMPUTE_UNITS)))
#endif

kernel void sort_data(
		global data_t* restrict  buffer1,
		global data_t* restrict  buffer2,
		int num_data
#ifdef GENERALIZED_START_SIZE
		,
		int start_chunk_size
#endif
		)
// -------------------------------------------------------
{
#if KNOB_WORK_ITEMS > 1
	const int tid = get_local_id(0);
#else
	const int tid = 0;
#endif

#if KNOB_WORK_GROUPS > 1
	const int startbuffer = get_group_id(0) * num_data;
#else
	const int startbuffer = 0;
#endif


	global data_t* in_ptr  = &buffer1[startbuffer];
	global data_t* out_ptr = &buffer2[startbuffer];


	// Apply merge sort using local memory if start_chunk_size is 1.
	//
#if LOCAL_SORT_SIZE > 1

#ifdef GENERALIZED_START_SIZE
	if(start_chunk_size <= 1)
#endif
	{
		// LOCAL_SORT_SIZE needs to divide num_data
		for(int left = 0; left < num_data; left += LOCAL_SORT_SIZE)
		{
			local_merge_sort(in_ptr, out_ptr, left, tid);
		}
		{
			global data_t* temp_ptr = in_ptr;
			in_ptr  = out_ptr;
			out_ptr = temp_ptr;
		}
	}
#endif // LOCAL_SORT_SIZE



	// Loop over chunk size (starts at size := LOCAL_SORT_SIZE, then size := size*2 ).
	// If the kernel is not generalized to the start size, we can use the log of the chunk size as an small optimization.
	//
#ifdef GENERALIZED_START_SIZE
	for(int chunk_size = (start_chunk_size <= 1 ? LOCAL_SORT_SIZE : start_chunk_size);
			chunk_size <= num_data-1;
			chunk_size = (chunk_size << 1))
	{
#else
	for(int log_chunk_size = KNOB_LOCAL_SORT_LOGSIZE;
			(1 << log_chunk_size) <= num_data-1;
			log_chunk_size++)
	{
		const int chunk_size = (1 << log_chunk_size);

#endif // GENERALIZED_START_SIZE



		// If chunk_size == 1, we can use a slightly simplified code to merge the chunks.
		//
#if LOCAL_SORT_SIZE <= 1 && KNOB_SPECIAL_CASE_1 == 1
		if(chunk_size == 1)
		{
			//const int loopblock = max(1, (num_data/2)/KNOB_WORK_ITEMS) * 2;
			const int loopblock = max(1, (num_data >> 1) / KNOB_WORK_ITEMS) << 1;
			const int loopstart = tid * loopblock;
			const int loopend   = (tid==KNOB_WORK_ITEMS-1) ? (num_data-1) : (min((tid+1) * loopblock, num_data-1));

			for(int left = loopstart; left < loopend; left += 2)
			{
				data_t left_val  = in_ptr[left];
				data_t right_val = in_ptr[left+1];

				out_ptr[left]    = min(left_val, right_val);
				out_ptr[left+1]  = max(left_val, right_val);
			}
		}
		else
#endif

		{
			// Divide the chunks among work-items
			//
			const int loopstep  = chunk_size << 1; // * 2
#ifdef GENERALIZED_START_SIZE
			const int loopblock = max(1, (num_data/loopstep)/KNOB_WORK_ITEMS) * loopstep;
#else
			const int loopblock = max(1, (num_data >> (log_chunk_size+1)) / KNOB_WORK_ITEMS) << (log_chunk_size+1);
#endif
			const int loopstart = tid * loopblock;
			const int loopend   = (tid==KNOB_WORK_ITEMS-1) ? (num_data-1) : (min((tid+1) * loopblock, num_data-1));
			

			// Loop over each chunk and merge them 2 by 2
			//
			for(int left = loopstart; left < loopend; left += loopstep)
			{
				const int mid   = min(left + chunk_size - 1,   num_data-1);
				const int right = min(left + 2*chunk_size - 1, num_data-1);

				global_merge_sort(in_ptr, out_ptr, left, mid, right);


			} // for each chunk

			
			// Note: for odd-sized arrays, the last number is ignored in the first iteration,
			// so we need to copy it from the in buffer to the out buffer.
			// We choose to pad the input data instead, but the padding size is negligible compared to the input size.
		}


		// Swap pointers
		//
		global data_t* temp_ptr = in_ptr;
		in_ptr  = out_ptr;
		out_ptr = temp_ptr;

#if KNOB_WORK_ITEMS > 1
		barrier(CLK_GLOBAL_MEM_FENCE);
#endif

	} // for each size of chunk


}

//#endif // __OPENCL_VERSION__





