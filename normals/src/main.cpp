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
 * Description: Normal estimation OpenCL benchmark.
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

#include "compute_normals.h"
#include "knobs.h"

using namespace std;



//---------------------------------------------
// Constants
//---------------------------------------------

#define CL_FILE_NAME   "normals.cl"
#define AOCX_FILE_NAME "normals.aocx"

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

	clContext->kernels[0] = clCreateKernel(clContext->program, "computeNmap_v3", &err);
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
			cout << "Usage: " << argv[0] << " [platform] [num_runs]" << endl;
			exit(EXIT_SUCCESS);
		}
	}


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


	// *********************
	//  Read data from files
	// *********************


	cout << "Reading files" << endl;



	int rows = 480;
	int cols = 640;

	AlignedArray<float> h_vmap(cols*rows*3);
	AlignedArray<float> h_nmap(cols*rows*3);

	{
		ifstream file("data/vmap.bin", ifstream::binary);
		ExitError(file.is_open());
		file.read((char*)h_vmap.data, cols*rows*3*sizeof(float));
	}
	{
		ifstream file("data/nmap.bin", ifstream::binary);
		ExitError(file.is_open());
		file.read((char*)h_nmap.data, cols*rows*3*sizeof(float));
	}


	int total_size = cols * rows;

	cout << "Size: " << cols << "x" << rows << endl;


	cout << "Files read" << endl;



	// *********************
	//  Initialize Memory
	// *********************


	cl_mem d_vmap = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float) * (total_size) * 3, NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));

	cl_mem d_output = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(float) * (total_size) * 3, NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));

	// Filling for debug purposes only
	//float fill_value = 0;
	//err = clEnqueueFillBuffer(command_queue, d_output, &fill_value, sizeof(float), 0, sizeof(float)*total_size*3, 0, NULL, NULL);
	//ExitError(checkErr(err, "Failed to fill output buffer!"));


	err = clEnqueueWriteBuffer(command_queue, d_vmap, CL_TRUE, 0, sizeof(float)*total_size*3, (void*)h_vmap.data, 0, NULL, NULL);
	ExitError(checkErr(err, "Failed to write data to the device!"));





	// *********************
	//  Run the program
	// *********************




	cout << "Running the program " << num_runs << " time" << (num_runs>1?"s.":".") << endl;


	size_t localWorkSize[1] = {KNOB_WORK_ITEMS};
	size_t workSize[1]      = {TOTAL_WORK_ITEMS};


	double totalTime = 0;
	double dryRunTime = 0;

	double kernel_time[clContext.events.size()];
	for(unsigned int i = 0; i < clContext.events.size(); i++){ kernel_time[i] = 0.0; }
	cl_ulong k_time_start, k_time_end;
	double k_temp, k_total_time = 0.0;


	for(int irun = 0; irun < num_runs+1; irun++)
	{
		// --- irun == 0: dry run


		// Set Arguments for kernel
		// 
		clSetKernelArg(kernel[0], 0, sizeof(cl_mem), (void*)&d_vmap);
		clSetKernelArg(kernel[0], 1, sizeof(cl_mem), (void*)&d_output);
		clSetKernelArg(kernel[0], 2, sizeof(int), (void*)&cols);
		clSetKernelArg(kernel[0], 3, sizeof(int), (void*)&rows);



		
		auto startTime = chrono::high_resolution_clock::now();

		{
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
	//cout << "Total time: " << totalTime << " ms" << endl;
	
	for(unsigned int i = 0; i < clContext.events.size(); i++)
	{
		kernel_time[i] /= num_runs;
		k_total_time += kernel_time[i];

		cout << "Kernel " << i+1 << ": " << kernel_time[i] << " ms" << endl;
	}
	cout << "Total time: " << k_total_time << " ms" << endl;





	// Copy result from device to host
	//
	AlignedArray<float> h_output(total_size * 3);
	clEnqueueReadBuffer(command_queue, d_output, CL_TRUE, 0, sizeof(float)*total_size*3, (void*)h_output.data, 0, NULL, NULL);
	ExitError(checkErr(err, "Failed to read data from the device!"));



	// Verification step
	//
	
	AlignedArray<float>& h_nmaps = h_nmap;

	bool passed = true;

	float epsilon = 0.0001;
	for(int i = 0; i < total_size && passed; i++)
	{
		bool nmaps_isnan  = std::isnan(h_nmaps[i*3]);
		bool output_isnan = std::isnan(h_output[i*3]);

		if( (nmaps_isnan && !output_isnan) || (!nmaps_isnan && output_isnan) )
		{
			passed = false;
		}
		else if(!nmaps_isnan && !output_isnan)
		{
			bool passed0 = std::abs(h_nmaps[i*3+0] - h_output[i*3+0]) < epsilon;
			bool passed1 = std::abs(h_nmaps[i*3+1] - h_output[i*3+1]) < epsilon;
			bool passed2 = std::abs(h_nmaps[i*3+2] - h_output[i*3+2]) < epsilon;

			passed = passed && passed0 && passed1 && passed2;
		}

		if(!passed)
		{
			cout << "\nground_truth |  calculated   at " << i << endl;
			for(int j = 0; j < 3; j++)
			{
				cout << h_nmaps[i*3+j] << " | " << h_output[i*3+j] << endl;
			}
			cout << endl;

			//debug
			cout << h_vmap[(i)*3+0] << " " << h_vmap[(i)*3+1] << " " << h_vmap[(i)*3+2] << endl << endl;
		}
	}

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
	clReleaseMemObject(d_vmap);
	clReleaseMemObject(d_output);


	return 0;
}


