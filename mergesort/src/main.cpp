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
 * Filename: main.cpp
 * Version: 1.0
 * Description: Merge sort OpenCL benchmark.
 * Author: Quentin Gautier
 */


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <malloc.h>

#include <iostream>
#include <fstream>

#include <chrono>
#include <algorithm>

#include "mergesort.h"
//#include "mergesort.cl"
#include "knobs.h"

using namespace std;



//---------------------------------------------
// Constants
//---------------------------------------------


#define CL_FILE_NAME   "mergesort.cl"
#define AOCX_FILE_NAME "mergesort.aocx"

#define AOCL_ALIGNMENT 64




//---------------------------------------------
// Type definitions
//---------------------------------------------

template<class T>
struct AlignedArray
{
	AlignedArray(size_t numElts){ data = (T*) memalign( AOCL_ALIGNMENT, sizeof(T) * numElts ); }
	~AlignedArray(){ free(data); }

	T& operator[](size_t idx){ return data[idx]; }
	const T& operator[](size_t idx) const { return data[idx]; }

	T* data;
};


//---------------------------------------------
// Functions
//---------------------------------------------


//---------------------------------------------
int gcd(int a, int b)
//---------------------------------------------
{
	while(true)
	{
		if (a == 0) return b;
		b %= a;
		if (b == 0) return a;
		a %= b;
	}
}

//---------------------------------------------
int lcm(int a, int b)
//---------------------------------------------
{
	int div = gcd(a, b);
	return div!=0 ? (a / div * b) : 0;
}


//---------------------------------------------
/// Initialize the OpenCL context and compile the kernels.
//---------------------------------------------
bool init_opencl(ClContext* clContext, cl_device_type device_type = CL_DEVICE_TYPE_ALL)
//---------------------------------------------
{
	cout << "Setting up OpenCL..." << endl;


	int err;

	vector<cl_platform_id> platform_ids;
	vector<cl_device_id> device_ids;
	cl_context context;


	// Get platform and devices
	//
	cl_uint num_platforms;
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	ReturnError(checkErr(err, "Failed to get number of platforms!"));

	cout << num_platforms << " platforms:" << endl;

	platform_ids.resize(num_platforms);

	err = clGetPlatformIDs(num_platforms, platform_ids.data(), NULL);
	ReturnError(checkErr(err, "Failed to get platform ID!"));

	for(cl_uint plat = 0; plat < num_platforms; plat++)
	{
		size_t sz;
		err = clGetPlatformInfo(platform_ids[plat], CL_PLATFORM_NAME, 0, NULL, &sz);
		ReturnError(checkErr(err, "Failed to get size of platform name!"));

		char* name = new char[sz];
		err = clGetPlatformInfo(platform_ids[plat], CL_PLATFORM_NAME, sz, name, NULL);
		ReturnError(checkErr(err, "Failed to get platform name!"));

		cout << "  - " << name << endl;


		cl_uint num_devices;
		clGetDeviceIDs(platform_ids[plat], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
		ReturnError(checkErr(err, "Failed to get number of devices!"));

		cout << "    with " << num_devices << " device(s):" << endl;

		device_ids.resize(num_devices);
		err = clGetDeviceIDs(platform_ids[plat], CL_DEVICE_TYPE_ALL, num_devices, device_ids.data(), NULL);
		ReturnError(checkErr(err, "Failed to get devices!"));

		for(cl_uint i = 0; i < num_devices; i++)
		{
			size_t sz;
			clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, 0, NULL, &sz);
			ReturnError(checkErr(err, "Failed to get size of device name!"));

			char* name = new char[sz];
			clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, sz, name, NULL);
			ReturnError(checkErr(err, "Failed to get device name!"));

			cout << "      - " << name << endl;

			delete[] name;
		}

	}






	// Connect to a compute device

	
	for(cl_uint plat = 0; plat < num_platforms; plat++)
	{
		err = clGetDeviceIDs(platform_ids[plat], device_type, 1, device_ids.data(), NULL);
		if(err == CL_SUCCESS){ break; }
	}	
	ReturnError(checkErr(err, "Failed to find a device!"));

	{
		size_t sz;
		clGetDeviceInfo(device_ids[0], CL_DEVICE_NAME, 0, NULL, &sz);
		ReturnError(checkErr(err, "Failed to get size of device name!"));

		char* name = new char[sz];
		clGetDeviceInfo(device_ids[0], CL_DEVICE_NAME, sz, name, NULL);
		ReturnError(checkErr(err, "Failed to get device name!"));

		cout << "Using " << name << endl;

		delete[] name;
	}

	// Create a compute context
	context = clCreateContext(0, 1, device_ids.data(), NULL, NULL, &err);
	ReturnError(checkErr(err, "Failed to create a compute context!"));


	// debug
	{
		clGetDeviceInfo(device_ids[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(clContext->maxWorkItems), clContext->maxWorkItems, NULL);
		cout << "Max work items: "
				<< clContext->maxWorkItems[0] << " "
				<< clContext->maxWorkItems[1] << " "
				<< clContext->maxWorkItems[2] << endl;
	}


	// Set contexts, build kernels, and create queues/events

	clContext->device = device_ids[0];
	clContext->context = context;
	clContext->device_type = device_type;




	// Read the kernel file

	std::ifstream file;
	if(device_type & CL_DEVICE_TYPE_ACCELERATOR)
	{ file.open(AOCX_FILE_NAME); }
	else
	{ file.open(CL_FILE_NAME); }
	ReturnError(checkErr(file.is_open() ? CL_SUCCESS:-1, "Cannot open file"));

	std::string prog(
			std::istreambuf_iterator<char>(file),
			(std::istreambuf_iterator<char>()));

	const char* prog_source = prog.c_str();
	const size_t prog_length = prog.size();

	// Create the compute program from the source buffer

	if(device_type & CL_DEVICE_TYPE_ACCELERATOR)
	{
		clContext->program = clCreateProgramWithBinary(clContext->context, 1, &clContext->device, &prog_length, (const unsigned char**)&prog_source, NULL, &err);
	}
	else
	{
		clContext->program = clCreateProgramWithSource(clContext->context, 1, &prog_source, NULL, &err);
	}
	ReturnError(checkErr(err, "Failed to create compute program!"));


	// Build the program executable

	err = clBuildProgram(clContext->program, 0, NULL, "-I .", NULL, NULL);
	{
		size_t logSize;
		checkErr(clGetProgramBuildInfo(clContext->program, clContext->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize),
				"Get builg log size");

		vector<char> buildLog(logSize);
		checkErr(clGetProgramBuildInfo(clContext->program, clContext->device, CL_PROGRAM_BUILD_LOG, logSize, buildLog.data(), NULL),
				"clGetProgramBuildInfo");

		string log(buildLog.begin(), buildLog.end());
		std::cout
		<< "\n----------------------Kernel build log----------------------\n"
		<< log
		<< "\n------------------------------------------------------------\n"
		<< std::endl;
	}
	ReturnError(checkErr(err, "Failed to build program executable!"));


	// Create kernels

	int numKernels = 1;

	clContext->kernels.resize(numKernels);

	clContext->kernels[0] = clCreateKernel(clContext->program, "sort_data", &err);
	ReturnError(checkErr(err, "Failed to create kernel!"));
	//clContext->kernels[1] = clCreateKernel(clContext->program, "kernel2", &err);
	//ReturnError(checkErr(err, "Failed to create kernel!"));



	// Create events

	clContext->events.resize(numKernels);

	// Create queues
	clContext->queues.resize(numKernels);
	for(unsigned int i = 0; i < clContext->queues.size(); i++)
	{
		clContext->queues[i] = clCreateCommandQueue(clContext->context, clContext->device, CL_QUEUE_PROFILING_ENABLE, &err);
		ReturnError(checkErr(err, "Failed to create a command queue!"));
	}




	return true;
}



//---------------------------------------------
int main(int argc, char ** argv)
//---------------------------------------------
{
	if(argc >= 2)
	{
		if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			cout << "Usage: " << argv[0] << " [platform] [num_runs] [data_size]" << endl;
			exit(EXIT_SUCCESS);
		}
	}

	srand(6442);

	int err;
	bool ferr;


	// *********************
	//  Initialize OpenCL
	// *********************

	ClContext clContext;

	// Default device is CPU
	cl_device_type device_type = CL_DEVICE_TYPE_CPU;

	if(argc >= 2)
	{
		if(strcmp(argv[1], "fpga") == 0)
		{
			device_type = CL_DEVICE_TYPE_ACCELERATOR;
		}
		else if(strcmp(argv[1], "gpu") == 0)
		{
			device_type = CL_DEVICE_TYPE_GPU;
		}
		else if(strcmp(argv[1], "cpu") == 0)
		{
			device_type = CL_DEVICE_TYPE_CPU;
		}
		else
		{
			cerr << "Warning! Device not recognized, using CPU" << endl;
		}
	}

	if(!init_opencl(&clContext, device_type)){ exit(EXIT_FAILURE); }


	cl_context context             = clContext.context;
	cl_command_queue command_queue = clContext.queues[0];
	cl_kernel* kernel              = clContext.kernels.data();
	cl_device_id device_id         = clContext.device;


	
	int num_runs = 1;
	if(argc >= 3)
	{
		num_runs = atoi(argv[2]);
	}

	int data_size = 1024;
	if(argc >= 4)
	{
		data_size = atoi(argv[3]);
	}

	// *********************
	//  Generate data
	// *********************

	int padding_size = data_size & 1; // padding at least so that number is even

	//int multiple = lcm(LOCAL_SORT_SIZE, KNOB_WORK_GROUPS); // size needs to divide both values
	int multiple = LOCAL_SORT_SIZE * KNOB_WORK_GROUPS; // size needs to divide LOCAL_SORT_SIZE for each work group

	if( multiple > 1 && (data_size % multiple != 0) ) // if size does not divide, calculate padding
	{
		padding_size = multiple - (data_size % multiple);
	}

	int total_size = data_size + padding_size;


	// Generate random input
	//
	AlignedArray<data_t> h_input(total_size);
	for(int i = 0; i < data_size; i++)
	{
		h_input[i] = rand() % data_size;
		//cout << h_input[i] << " ";
	}
	//cout << endl;
	
	// Padding values
	//
	for(int i = data_size; i < total_size; i++)
	{
		h_input[i] = std::numeric_limits<data_t>::max();
	}



	cout << "Size: " << data_size << " (" << total_size << ")" << endl;




	// *********************
	//  Initialize Memory
	// *********************


	cl_mem d_buffer1 = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(data_t) * total_size, NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));

	cl_mem d_buffer2 = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(data_t) * total_size, NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));




	// *********************
	//  Run the program
	// *********************




	cout << "Running the program " << num_runs << " time" << (num_runs>1?"s.":".") << endl;


	double totalTime = 0;
	double dryRunTime = 0;

	double kernel_time[clContext.events.size()];
	for(unsigned int i = 0; i < clContext.events.size(); i++){ kernel_time[i] = 0.0; }
	cl_ulong k_time_start, k_time_end;
	double k_temp, k_total_time = 0.0;


	// Run the kernel multiple times to measure the running time
	//
	for(int irun = 0; irun < num_runs+1; irun++)
	{
		// *** irun == 0: dry run ***


		// Set Arguments for kernel
		//
		int size_per_group = total_size / KNOB_WORK_GROUPS;
		int start_chunk     = 1;
		clSetKernelArg(kernel[0], 0, sizeof(cl_mem), (void*)&d_buffer1);
		clSetKernelArg(kernel[0], 1, sizeof(cl_mem), (void*)&d_buffer2);
		clSetKernelArg(kernel[0], 2, sizeof(int), (void*)&size_per_group);
#ifdef GENERALIZED_START_SIZE
		clSetKernelArg(kernel[0], 3, sizeof(int), (void*)&start_chunk);
#endif

		size_t localWorkSize[1] = {KNOB_WORK_ITEMS};
		size_t workSize[1]      = {TOTAL_WORK_ITEMS};


		// Copy the input data
		//
		err = clEnqueueWriteBuffer(command_queue, d_buffer1, CL_TRUE, 0, sizeof(data_t) * total_size, (void*)h_input.data, 0, NULL, NULL);
		ExitError(checkErr(err, "Failed to write data to the device!"));

		
		auto startTime = chrono::high_resolution_clock::now();


		// If there are multiple work groups, we need two iterations
		//
		for(int ikernel = 0; ikernel < (KNOB_WORK_GROUPS>1? 2: 1); ikernel++)
		{
			// Run the kernel
			cl_int err = clEnqueueNDRangeKernel(
					command_queue, kernel[0], 1, NULL,
					workSize, localWorkSize, 0, NULL, &clContext.events[0]);
			clFinish(command_queue);
			
			ExitError(checkErr(err, "Failed to execute kernel!"));

			// Get kernel execution times
			if(irun != 0)
			{
				for(unsigned int i = 0; i < clContext.events.size(); i++)
				{
					clGetEventProfilingInfo(clContext.events[i], CL_PROFILING_COMMAND_START, sizeof(k_time_start), &k_time_start, NULL);
					clGetEventProfilingInfo(clContext.events[i], CL_PROFILING_COMMAND_END, sizeof(k_time_end), &k_time_end, NULL);
					k_temp = k_time_end - k_time_start;
					kernel_time[i] += (k_temp / 1000000.0);
				}
			}


			// Prepare the second iteration of the kernel
			if(ikernel == 0 && KNOB_WORK_GROUPS > 1)
			{
				// Which buffer contains the output?
				int buffer1_index = 1;
				int log_local_sort = std::max(1, KNOB_LOCAL_SORT_LOGSIZE);
				if( ( (int)ceil(log2((float)size_per_group) + (log_local_sort-1)) & 1) == 0) // if result is in first buffer
				{ buffer1_index = 0; }


				size_per_group  = total_size;
				start_chunk     = total_size / KNOB_WORK_GROUPS;
				clSetKernelArg(kernel[0], buffer1_index, sizeof(cl_mem), (void*)&d_buffer1);
				clSetKernelArg(kernel[0], 1-buffer1_index, sizeof(cl_mem), (void*)&d_buffer2);
				clSetKernelArg(kernel[0], 2, sizeof(int), (void*)&size_per_group);
				clSetKernelArg(kernel[0], 3, sizeof(int), (void*)&start_chunk);


				localWorkSize[0] = KNOB_WORK_ITEMS;
				workSize[0]      = KNOB_WORK_ITEMS;
			}

		}
		
		auto endTime = chrono::high_resolution_clock::now();

		
		if(irun != 0) // not dry run
		{
			totalTime += chrono::duration <double, milli> (endTime - startTime).count();
		}
		else
		{
			dryRunTime = chrono::duration <double, milli> (endTime - startTime).count();
		}

		cout << "*";
		cout << flush;
	}
	cout << endl;

	totalTime /= num_runs;
	
	cout << "Dry run time: " << dryRunTime << " ms" << endl;
	cout << "Wall-clock time: " << totalTime << " ms" << endl;
	
	for(unsigned int i = 0; i < clContext.events.size(); i++)
	{
		kernel_time[i] /= num_runs;
		k_total_time += kernel_time[i];

		cout << "Kernel " << i+1 << ": " << kernel_time[i] << " ms" << endl;
	}
	cout << "Total time: " << k_total_time << " ms" << endl;





	// Copy result from device to host
	//
	AlignedArray<data_t> h_output(total_size);
	
	int log_local_sort = std::max(1, KNOB_LOCAL_SORT_LOGSIZE);

	if( ( (int)ceil(log2((float)total_size) + (log_local_sort-1)) & 1) == 0) // if result is in first buffer
	{
		err = clEnqueueReadBuffer(
				command_queue, d_buffer1, CL_TRUE, 0,
				sizeof(data_t)*total_size, (void*)h_output.data, 0, NULL, NULL);
	}
	else
	{
		err = clEnqueueReadBuffer(
				command_queue, d_buffer2, CL_TRUE, 0,
				sizeof(data_t)*total_size, (void*)h_output.data, 0, NULL, NULL);
	}
	ExitError(checkErr(err, "Failed to read data from the device!"));



	// Verification step
	//

	vector<data_t> ground_truth(h_input.data, h_input.data + data_size);
	std::sort(ground_truth.begin(), ground_truth.end());

	//for(int i = 0; i < total_size; i++){ cout << h_output[i] << " "; }
	//cout << endl;

	bool passed = std::equal(h_output.data, h_output.data+data_size, ground_truth.begin());

	cout << "Verification: " << (passed? "Passed": "Failed") << endl;





	// Cleanup memory
	//
	//   free device memory
	for (unsigned int ki = 0; ki < clContext.kernels.size(); ki++)
   	{
		clReleaseKernel(kernel[ki]);
	}
	clReleaseProgram(clContext.program);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	clReleaseMemObject(d_buffer1);
	clReleaseMemObject(d_buffer2);


	return 0;
}
