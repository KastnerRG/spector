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
 * Filename: histogram.cpp
 * Version: 1.0
 * Description: Histogram calculation OpenCL benchmark.
 * Author: Quentin Gautier
 */


#include <cmath>
#include <limits>
#include <iostream>

#include <chrono>

#include "histogram.h"
#include "params.h"



using namespace std;


// --------------------------------------------
void histogram_cpu(const std::vector<data_t>& data, std::vector<unsigned>& histogram)
// --------------------------------------------
{
	histogram.clear();

	histogram.resize(256, 0);

	for(auto val : data)
	{
		histogram[val]++;
	}
}




// --------------------------------------------
bool histogram_cl(
		ClContext& context,
		const std::vector<data_t>& data,
		std::vector<unsigned>& histogram)
// --------------------------------------------
{
	cl_int err;

	if(context.kernels.size() < 1){ return false; }
	cl_kernel kernel_hist = context.kernels[0];
	cl_kernel kernel_accum = context.kernels[1];

	for(unsigned int i = 0; i < context.events.size(); i++)
	{
		clReleaseEvent(context.events[i]);
	}

	histogram.clear();
	histogram.resize(256, 0);
	vector<unsigned int> histogramTemp(256 * TOTAL_WORK_ITEMS, 0);


	// ---------------
	// Set up memory
	// ---------------

	cl_mem data_d = clCreateBuffer(context.context, CL_MEM_READ_ONLY, data.size() * sizeof(data_t), NULL, &err);
	ReturnError(checkErr(err, "Failed to allocate memory!"));
	cl_mem hist_d = clCreateBuffer(context.context, CL_MEM_READ_WRITE, histogram.size() * sizeof(unsigned), NULL, &err);
	ReturnError(checkErr(err, "Failed to allocate memory!"));
	cl_mem histtemp_d = clCreateBuffer(context.context, CL_MEM_READ_WRITE, histogramTemp.size() * sizeof(unsigned), NULL, &err);
	ReturnError(checkErr(err, "Failed to allocate memory!"));

	err = clEnqueueWriteBuffer(context.queues[0], data_d, CL_FALSE, 0, data.size() * sizeof(data_t), data.data(), 0, NULL, NULL);
	ReturnError(checkErr(err, "Failed to copy structure to device!"));
	err = clEnqueueWriteBuffer(context.queues[0], hist_d, CL_FALSE, 0, histogram.size() * sizeof(unsigned), histogram.data(), 0, NULL, NULL);
	ReturnError(checkErr(err, "Failed to copy structure to device!"));
	err = clEnqueueWriteBuffer(context.queues[0], histtemp_d, CL_FALSE, 0, histogramTemp.size() * sizeof(unsigned), histogramTemp.data(), 0, NULL, NULL);
	ReturnError(checkErr(err, "Failed to copy structure to device!"));


	int numData = data.size() / TOTAL_WORK_ITEMS;

	// ---------------
	// Set kernel args
	// ---------------

	err  = clSetKernelArg(kernel_hist, 0, sizeof(cl_mem), &data_d);
#if TOTAL_WORK_ITEMS == 1 || KNOB_ACCUM_SMEM == 1
	err |= clSetKernelArg(kernel_hist, 1, sizeof(cl_mem), &hist_d);
#else
	err |= clSetKernelArg(kernel_hist, 1, sizeof(cl_mem), &histtemp_d);
#endif
	err |= clSetKernelArg(kernel_hist, 2, sizeof(numData), &numData);
	ReturnError(checkErr(err, "Failed to set kernel arguments!"));


	// ---------------
	// Launch kernel
	// ---------------

	auto startTime = chrono::high_resolution_clock::now();


	size_t local_work = KNOB_NUM_WORK_ITEMS;
	size_t global_work = TOTAL_WORK_ITEMS;

	err = clEnqueueNDRangeKernel(context.queues[0], kernel_hist, 1, NULL, &global_work, &local_work, 0, NULL, &context.events[0]);



	ReturnError(checkErr(err, "Failed to execute kernel!"));

	clFinish(context.queues[0]);




#if TOTAL_WORK_ITEMS > 1 && KNOB_ACCUM_SMEM == 0
	err  = clSetKernelArg(kernel_accum, 0, sizeof(cl_mem), &histtemp_d);
	err |= clSetKernelArg(kernel_accum, 1, sizeof(cl_mem), &hist_d);
	ReturnError(checkErr(err, "Failed to set kernel arguments!"));

	local_work = 1;
	global_work = 1;
	err = clEnqueueNDRangeKernel(context.queues[1], kernel_accum, 1, NULL, &global_work, &local_work, 0, NULL, &context.events[1]);

	ReturnError(checkErr(err, "Failed to execute kernel!"));

	clFinish(context.queues[1]);

#endif




	auto endTime = chrono::high_resolution_clock::now();

	// ---------------
	// Copy data back
	// ---------------

	err = clEnqueueReadBuffer(context.queues[0], hist_d, CL_TRUE, 0, histogram.size() * sizeof(unsigned), histogram.data(), 0, NULL, NULL);
	ReturnError(checkErr(err, "Failed to read output array!"));


	// ---------------
	// Display time
	// ---------------
	auto totalTime = chrono::duration <double, milli> (endTime - startTime).count();
	cout << "Total time: " << totalTime << " ms" << endl;


	
	// ---------------
	// Free memory
	// ---------------
	clReleaseMemObject(data_d);
	clReleaseMemObject(hist_d);
	clReleaseMemObject(histtemp_d);


	return true;
}








