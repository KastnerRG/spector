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
 * Filename: opencl_utils.h
 * Version: 1.0
 * Description: Functions and types for OpenCL
 * Author: Quentin Gautier
 *
 */


#include <iostream>
#include <fstream>
#include <vector>

#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS

#include <CL/cl.h>



//*********************************************
// Pre-processor macros
//*********************************************

#define ReturnError(x) if(!(x)){ return false; }
#define ExitError(x)   if(!(x)){ exit(1); }
#define checkErr(x, y) checkErr_(x, y, __LINE__, __FILE__)


//*********************************************
// Namespace
//*********************************************
namespace spector
{

//*********************************************
// Type definitions
//*********************************************

struct ClContext
{
	cl_context context;
	cl_device_id device;
	cl_device_type device_type;
	size_t maxWorkItems[3];

	std::vector<cl_command_queue> queues;

	std::vector<cl_event> events;

	cl_program program;
	std::vector<cl_kernel> kernels;
};



//*********************************************
// Functions
//*********************************************

//---------------------------------------------
inline bool checkErr_(cl_int err, const char * name, int line = -1, const char* file = NULL)
//---------------------------------------------
{
	if (err != CL_SUCCESS)
	{
		std::cerr << "ERROR: " << name << " (" << err << ")" << std::endl;
		if(line >= 0){ std::cerr << "\tAt line " << line << "\n"; }
		if(file){ std::cerr << "\tIn file \"" << file << "\"" << std::endl; }
		return false;
	}

	return true;
}


//---------------------------------------------
bool init_opencl(
		ClContext* clContext,
		const std::vector<std::string>& kernel_names,
		cl_device_type device_type = CL_DEVICE_TYPE_ALL,
		const char* cl_filename = "",
		const char* aocx_filename = "",
		bool verbose = true)
//---------------------------------------------
{
	if(verbose)
	{ std::cout << "Setting up OpenCL..." << std::endl; }


	int err;

	std::vector<cl_platform_id> platform_ids;
	std::vector<cl_device_id> device_ids;
	cl_context context;


	// Get platform and devices
	//
	cl_uint num_platforms;
	err = clGetPlatformIDs(0, NULL, &num_platforms);
	ReturnError(checkErr(err, "Failed to get number of platforms!"));

	if(verbose)
	{ std::cout << num_platforms << " platforms:" << std::endl; }

	platform_ids.resize(num_platforms);

	err = clGetPlatformIDs(num_platforms, platform_ids.data(), NULL);
	ReturnError(checkErr(err, "Failed to get platform ID!"));

	if(verbose)
	{
		for(cl_uint plat = 0; plat < num_platforms; plat++)
		{
			size_t plat_name_size;
			err = clGetPlatformInfo(platform_ids[plat], CL_PLATFORM_NAME, 0, NULL, &plat_name_size);
			ReturnError(checkErr(err, "Failed to get size of platform name!"));

			std::vector<char> name(plat_name_size);
			err = clGetPlatformInfo(platform_ids[plat], CL_PLATFORM_NAME, plat_name_size, name.data(), NULL);
			ReturnError(checkErr(err, "Failed to get platform name!"));

			std::cout << "  - " << name.data() << std::endl;


			cl_uint num_devices;
			clGetDeviceIDs(platform_ids[plat], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
			ReturnError(checkErr(err, "Failed to get number of devices!"));

			std::cout << "    with " << num_devices << " device(s):" << std::endl;

			device_ids.resize(num_devices);
			err = clGetDeviceIDs(platform_ids[plat], CL_DEVICE_TYPE_ALL, num_devices, device_ids.data(), NULL);
			ReturnError(checkErr(err, "Failed to get devices!"));

			for(cl_uint i = 0; i < num_devices; i++)
			{
				size_t device_name_size;
				clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, 0, NULL, &device_name_size);
				ReturnError(checkErr(err, "Failed to get size of device name!"));

				std::vector<char> name(device_name_size);
				clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, device_name_size, name.data(), NULL);
				ReturnError(checkErr(err, "Failed to get device name!"));

				std::cout << "      - " << name.data() << std::endl;
			}

		}
	}




	// Connect to a compute device
	device_ids.resize(1);
	for(cl_uint plat = 0; plat < num_platforms; plat++)
	{
		err = clGetDeviceIDs(platform_ids[plat], device_type, 1, device_ids.data(), NULL);
		if(err == CL_SUCCESS){ break; }
	}	
	ReturnError(checkErr(err, "Failed to find a device!"));

	if(verbose)
	{
		size_t device_name_size;
		clGetDeviceInfo(device_ids[0], CL_DEVICE_NAME, 0, NULL, &device_name_size);
		ReturnError(checkErr(err, "Failed to get size of device name!"));
		
		std::vector<char> name(device_name_size);
		clGetDeviceInfo(device_ids[0], CL_DEVICE_NAME, device_name_size, name.data(), NULL);
		ReturnError(checkErr(err, "Failed to get device name!"));

		std::cout << "Using " << name.data() << std::endl;
	}

	// Create a compute context
	context = clCreateContext(0, 1, device_ids.data(), NULL, NULL, &err);
	ReturnError(checkErr(err, "Failed to create a compute context!"));


	// Get max work-items
	clGetDeviceInfo(device_ids[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(clContext->maxWorkItems), clContext->maxWorkItems, NULL);
	if(verbose)
	{
		std::cout << "Max work items: "
			<< clContext->maxWorkItems[0] << " "
			<< clContext->maxWorkItems[1] << " "
			<< clContext->maxWorkItems[2] << std::endl;
	}


	// Set contexts, build kernels, and create queues/events
	//
	
	clContext->device = device_ids[0];
	clContext->context = context;
	clContext->device_type = device_type;


	// Read the kernel file

	std::ifstream file;
	if(device_type & CL_DEVICE_TYPE_ACCELERATOR)
	{ file.open(aocx_filename); }
	else
	{ file.open(cl_filename); }
	ReturnError(checkErr(file.is_open() ? CL_SUCCESS:-1, "Cannot open kernel file"));

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
	if(err != CL_SUCCESS || verbose)
	{
		size_t logSize;
		checkErr(clGetProgramBuildInfo(clContext->program, clContext->device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize),
				"Get builg log size");

		std::vector<char> buildLog(logSize);
		checkErr(clGetProgramBuildInfo(clContext->program, clContext->device, CL_PROGRAM_BUILD_LOG, logSize, buildLog.data(), NULL),
				"clGetProgramBuildInfo");

		std::string log(buildLog.begin(), buildLog.end());
		std::cout
		<< "\n----------------------Kernel build log----------------------\n"
		<< log
		<< "\n------------------------------------------------------------\n"
		<< std::endl;
	}
	ReturnError(checkErr(err, "Failed to build program executable!"));


	// Create kernels

	size_t numKernels = kernel_names.size();

	clContext->kernels.resize(numKernels);

	for(size_t i = 0; i < numKernels; i++)
	{
		clContext->kernels[i] = clCreateKernel(clContext->program, kernel_names[i].c_str(), &err);
		ReturnError(checkErr(err, "Failed to create kernel!"));
	}



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


} // namespace






