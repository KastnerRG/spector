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
 * Description: Histogram calculation OpenCL benchmark.
 * Author: Quentin Gautier
 */


#include <cstdlib>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cstring>

#include <random>

#include "histogram.h"
#include "params.h"


using namespace std;



//---------------------------------
// Functions
//---------------------------------



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
		size_t maxItems[3];
		clGetDeviceInfo(device_ids[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxItems), maxItems, NULL);
		cout << "Max work items: " << maxItems[0] << " " << maxItems[1] << " " << maxItems[2] << endl;
	}


	// Set contexts, build kernels, and create queues/events

	clContext->device = device_ids[0];
	clContext->context = context;
	clContext->device_type = device_type;




	// Read the kernel file

	std::ifstream file;
	if(device_type & CL_DEVICE_TYPE_ACCELERATOR)
	{ file.open("histogram.aocx"); }
	else
	{ file.open("histogram.cl"); }
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

	int numKernels = 2;

	clContext->kernels.resize(numKernels);

	clContext->kernels[0] = clCreateKernel(clContext->program, "calculateHistogram", &err);
	ReturnError(checkErr(err, "Failed to create kernel!"));
#if TOTAL_WORK_ITEMS > 1 && KNOB_ACCUM_SMEM == 0
	clContext->kernels[1] = clCreateKernel(clContext->program, "accumulateHistograms", &err);
	ReturnError(checkErr(err, "Failed to create kernel!"));
#endif



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


// --------------------------------------------
/**
 * Read values from a binary file into a preallocated array.
 * Could be used to initialize the input data.
 **/
// --------------------------------------------
template<class T>
bool readData(const char* filename, T* data, int numData)
// --------------------------------------------
{
	ifstream file(filename);
	if(!file.is_open()){ return false; }

	file.read((char*)data, numData * sizeof(T));

	return true;
}


// --------------------------------------------
/**
 * Generate random values, mostly between 0 and 255.
 **/
// --------------------------------------------
template<class T>
void generateInputData(vector<T>& data, int numData)
// --------------------------------------------
{
	data.resize(numData);

	default_random_engine randeng(42);
	//default_random_engine randeng(time(0));

	normal_distribution<float> randdist(128, 43);


	for(int i = 0; i < numData; i++)
	{
		data[i] = static_cast<T>(randdist(randeng));
	}
}



// --------------------------------------------
int main(int argc, char **argv)
//---------------------------------------------
{
	// *********************
	//  Initialize OpenCL
	// *********************

	ClContext clContext;

	// Default device is FPGA
	cl_device_type device_type = CL_DEVICE_TYPE_ACCELERATOR;

	if(argc > 1)
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
			cerr << "Warning! Device not recognized, using FPGA" << endl;
		}
	}

	int num_runs = 1;
	if(argc > 2)
	{
		num_runs = atoi(argv[2]);
	}

	ReturnError(init_opencl(&clContext, device_type));


	// *********************
	//   Input data
	// *********************
	// TODO Input size as parameter
	vector<data_t> inputData;
	generateInputData(inputData, pow(2, 20)); // ~1,000,000 values

	vector<unsigned int> histogram;


	// *********************
	//   Compute histogram
	// *********************

	vector<double> kernel_times(clContext.events.size(), 0.0);
	double total_time = 0.0;

	// Dry run
	histogram_cl(clContext, inputData, histogram);

	for(int n = 0; n < num_runs; n++)
	{
		histogram_cl(clContext, inputData, histogram);

		// Get kernel execution times
		{	
			for(unsigned int i = 0; i < clContext.events.size(); i++)
			{
				if(i == 1 && (TOTAL_WORK_ITEMS == 1 || KNOB_ACCUM_SMEM == 1)){ break; }

				cl_ulong time_start, time_end;
				clGetEventProfilingInfo(clContext.events[i], CL_PROFILING_COMMAND_START, sizeof(time_start), &time_start, NULL);
				clGetEventProfilingInfo(clContext.events[i], CL_PROFILING_COMMAND_END, sizeof(time_end), &time_end, NULL);
				double time = (time_end - time_start) / 1000000.0;

				kernel_times[i] += time;

				total_time += time;
			}

		}
	}

	for(int i = 0; i < clContext.events.size(); i++)
	{
		kernel_times[i] /= num_runs;
		cout << "Kernel " << i << ": " << kernel_times[i] << " ms" << endl;
	}

	total_time /= num_runs;

	cout << "Total: " << total_time << " ms" << endl;



	// *********************
	//   Verify result
	// *********************

	vector<unsigned> histogram2;
	histogram_cpu(inputData, histogram2);

	bool same = true;

	if(histogram.size() != histogram2.size())
	{
		cerr << "Error: Histogram sizes do not match!" << endl;
		same = false;
	}

	for(unsigned i = 0; i < histogram.size() && same; i++)
	{
		auto val1 = histogram[i];
		auto val2 = histogram2[i];

		if(val1 != val2){ same = false; }
	}


//	cout << "Device: ";
//	for(auto val : histogram)
//	{
//		cout << val << " ";
//	}
//	cout << endl;
//	cout << "  Host: ";
//	for(auto val : histogram2)
//	{
//		cout << val << " ";
//	}
//	cout << endl;


	if(!same)
	{
		cout << "Verification: FAILED" << endl;
	}
	else
	{
		cout << "Verification: SUCCESS" << endl;
	}




	// *********************
	//   Cleanup memory
	// *********************
	for (unsigned int ki = 0; ki < clContext.kernels.size(); ki++)
   	{
		clReleaseKernel(clContext.kernels[ki]);
	}
	clContext.kernels.clear();
	for(auto& queue: clContext.queues)
	{
		clReleaseCommandQueue(queue);
	}
	clContext.queues.clear();
	clReleaseProgram(clContext.program);
	clReleaseContext(clContext.context);



	cout << "Program done." << endl;
	exit(EXIT_SUCCESS);
}






