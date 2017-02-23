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
 * Filename: my_bfs.cpp
 * Version: 1.0
 * Description: Breadth-first search OpenCL benchmark.
 * Author: Quentin Gautier
 *
 * Note: This program calls the methods defined in bfs_fpga.cl, which is a
 * derivative work released under the LGPL license provided in the LICENSE file.
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

#include "bfs.h"
#include "knobs.h"

using namespace std;



//---------------------------------------------
// Constants
//---------------------------------------------

#define CL_FILE_NAME   "bfs_fpga.cl"
#define AOCX_FILE_NAME "bfs_fpga.aocx"

#define AOCL_ALIGNMENT 64


//---------------------------------------------
// Type definitions
//---------------------------------------------

/// Represents one node in the graph
struct Node
{
	int starting;     //Index where the edges of the node start
	int no_of_edges;  //The degree of the node
};


/// Scoped array aligned in memory
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

	int numKernels = 2;

	clContext->kernels.resize(numKernels);

	clContext->kernels[0] = clCreateKernel(clContext->program, "kernel1", &err);
	ReturnError(checkErr(err, "Failed to create kernel!"));
	clContext->kernels[1] = clCreateKernel(clContext->program, "kernel2", &err);
	ReturnError(checkErr(err, "Failed to create kernel!"));



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
void bfs_cpu(
		int     no_of_nodes,
		Node*   h_graph_nodes,
		int     edge_list_size,
		int*    h_graph_edges,
		mask_t* h_graph_mask,
		mask_t* h_updating_graph_mask,
		mask_t* h_graph_visited,
		int*    h_cost_ref)
//---------------------------------------------
{
	mask_t not_over = 1;

	while(not_over)
	{
		not_over = 0;

		// Kernel 1
		for(int tid = 0; tid < no_of_nodes; tid++)
		{
			if (h_graph_mask[tid] == 1)
			{ 
				h_graph_mask[tid] = 0;

				int start = h_graph_nodes[tid].starting;
				int end   = h_graph_nodes[tid].no_of_edges + h_graph_nodes[tid].starting;

				for(int i = start; i < end; i++)
				{
					int id = h_graph_edges[i];

					if(!h_graph_visited[id])
					{
						h_cost_ref[id] = h_cost_ref[tid] + 1;
						h_updating_graph_mask[id] = 1;
					}
				}
			}		
		}

		// Kernel 2
  		for(int tid = 0; tid < no_of_nodes; tid++)
		{
			if (h_updating_graph_mask[tid] == 1)
			{
				h_graph_mask[tid]    = 1;
				h_graph_visited[tid] = 1;

				not_over = 1;

				h_updating_graph_mask[tid] = 0;
			}
		}
	}
}


//---------------------------------------------
void printUsage(const char* argv0)
//---------------------------------------------
{
	cout << "Usage: " << argv0 << " <filename> [platform] [num_runs]" << endl;
}


//---------------------------------------------
int main(int argc, char ** argv)
//---------------------------------------------
{
	if(argc < 2)
	{
		printUsage(argv[0]);
		exit(EXIT_FAILURE);
	}
	else if(argc >= 2)
	{
		if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
		{
			printUsage(argv[0]);
			exit(EXIT_SUCCESS);
		}
	}


	// *********************
	//  Initialize OpenCL
	// *********************

	ClContext clContext;

	// Default device is CPU
	cl_device_type device_type = CL_DEVICE_TYPE_CPU;

	if(argc > 2)
	{
		if(strcmp(argv[2], "fpga") == 0)
		{
			device_type = CL_DEVICE_TYPE_ACCELERATOR;
		}
		else if(strcmp(argv[2], "gpu") == 0)
		{
			device_type = CL_DEVICE_TYPE_GPU;
		}
		else if(strcmp(argv[2], "cpu") == 0)
		{
			device_type = CL_DEVICE_TYPE_CPU;
		}
		else
		{
			cerr << "Warning! Device not recognized, using CPU" << endl;
		}
	}

	if(!init_opencl(&clContext, device_type)){ exit(EXIT_FAILURE); }




	int num_runs = 1;
	if(argc >= 4)
	{
		num_runs = atoi(argv[3]);
	}
	cout << "Running the program " << num_runs << " time" << (num_runs!=1?"s.":".") << endl;







	cout << "Reading File" << endl;

	unsigned int no_of_nodes;
	unsigned int edge_list_size;

	
	//Read in Graph from a file
	//
	ifstream input_file(argv[1]);
	if(!input_file.is_open()){ cerr << "Unable to read \"" << argv[1] << "\"" << endl; exit(EXIT_FAILURE); }


	input_file >> no_of_nodes;

	cout << "Number of nodes:" << no_of_nodes << endl;
    
	AlignedArray<Node>    h_graph_nodes        (no_of_nodes);
	AlignedArray<mask_t>  h_graph_mask         (no_of_nodes);
	AlignedArray<mask_t>  h_updating_graph_mask(no_of_nodes);
	AlignedArray<mask_t>  h_graph_visited      (no_of_nodes);


	// Initialize the memory
	for(unsigned int i = 0; i < no_of_nodes; i++)
	{
		int start, edgeno;

		input_file >> start >> edgeno;

		h_graph_nodes[i].starting    = start; 
		h_graph_nodes[i].no_of_edges = edgeno;
		h_graph_mask[i]              = 0;
		h_updating_graph_mask[i]     = 0;
		h_graph_visited[i]           = 0;
	}

	// Read the source node from the file
	int source = 0;
	input_file >> source;

	// Set the source node as true in the masks
	h_graph_mask[source]    = 1;
	h_graph_visited[source] = 1;

	input_file >> edge_list_size;
	
	cout << "Number of edges:" << edge_list_size << endl;

	// Read the edges
	AlignedArray<int> h_graph_edges(edge_list_size);

	for(unsigned int i = 0; i < edge_list_size; i++)
	{
		int id, cost;
		input_file >> id;
		input_file >> cost;
		h_graph_edges[i] = id;
	}

	cout << "File read" << endl;



	cl_context context             = clContext.context;
	cl_command_queue command_queue = clContext.queues[0];
	cl_kernel* kernel              = clContext.kernels.data();
	cl_device_id device_id         = clContext.device;


	// Initialize OpenCL buffers
	//
	int err;

	cl_mem d_graph_nodes = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(Node) * no_of_nodes, NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));
	cl_mem d_graph_edges = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * edge_list_size, NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));
	cl_mem d_graph_mask          = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(mask_t) * no_of_nodes, NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));
	cl_mem d_updating_graph_mask = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(mask_t) * no_of_nodes, NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));
	cl_mem d_graph_visited       = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(mask_t) * no_of_nodes, NULL,  &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));

	// Output buffers
	AlignedArray<int> h_cost(no_of_nodes);
	AlignedArray<int> h_cost_ref(no_of_nodes);

	for(unsigned int i = 0; i < no_of_nodes; i++)
	{
		h_cost[i]     = -1;
		h_cost_ref[i] = -1;
	}
	h_cost[source]     = 0;
	h_cost_ref[source] = 0;

	// Allocate device memory for result
	cl_mem d_cost = clCreateBuffer(context, CL_MEM_READ_WRITE, sizeof(int) * no_of_nodes, NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));

	// Make a bool to check if the execution is over
	cl_mem d_over = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int), NULL, &err);
	ExitError(checkErr(err, "Failed to allocate memory!"));
	


	// Number of work-items
	size_t localWorkSize_1[1] = {KNOB_NUM_WORK_ITEMS_1}; //{no_of_nodes < 256? no_of_nodes: 256};
	size_t workSize_1[1]      = {(no_of_nodes/localWorkSize_1[0])*localWorkSize_1[0] + ((no_of_nodes%localWorkSize_1[0])==0?0:localWorkSize_1[0])}; // one dimensional Range

	size_t localWorkSize_2[1] = {KNOB_NUM_WORK_ITEMS_2}; //{no_of_nodes < 256? no_of_nodes: 256};
	size_t workSize_2[1]      = {(no_of_nodes/localWorkSize_2[0])*localWorkSize_2[0] + ((no_of_nodes%localWorkSize_2[0])==0?0:localWorkSize_2[0])}; // one dimensional Range

	cout << "localWorkSize_1 = " << localWorkSize_1[0] << "  workSize_1 = " << workSize_1[0] << endl;
	cout << "localWorkSize_2 = " << localWorkSize_2[0] << "  workSize_2 = " << workSize_2[0] << endl;


	int k = 0;
	
	// Timers
	//
	double totalTime = 0;
	double dryRunTime = 0;

	double kernel_time[clContext.events.size()];
	for(unsigned int i = 0; i < clContext.events.size(); i++){ kernel_time[i] = 0.0; }
	cl_ulong k_time_start, k_time_end;
	double k_temp, k_total_time = 0.0;


	// Run the algorithm multiple times
	//
	for(int irun = 0; irun < num_runs+1; irun++)
	{
		// *** irun == 0: dry run ***
		

		k = 0;
		int stop;



		// Copy data to device memory
		//
		clEnqueueWriteBuffer(command_queue, d_graph_nodes, CL_TRUE, 0, sizeof(Node) * no_of_nodes, h_graph_nodes.data, 0, NULL, NULL);
		ExitError(checkErr(err, "Failed to copy data to device!"));
		clEnqueueWriteBuffer(command_queue, d_graph_edges, CL_TRUE, 0, sizeof(int) * edge_list_size, h_graph_edges.data, 0, NULL, NULL);
		ExitError(checkErr(err, "Failed to copy data to device!"));
		clEnqueueWriteBuffer(command_queue, d_graph_mask, CL_TRUE, 0, sizeof(mask_t) * no_of_nodes, h_graph_mask.data, 0, NULL, NULL);
		ExitError(checkErr(err, "Failed to copy data to device!"));
		clEnqueueWriteBuffer(command_queue, d_updating_graph_mask, CL_TRUE, 0, sizeof(mask_t) * no_of_nodes, h_updating_graph_mask.data, 0, NULL, NULL);
		ExitError(checkErr(err, "Failed to copy data to device!"));
		clEnqueueWriteBuffer(command_queue, d_graph_visited, CL_TRUE, 0, sizeof(mask_t) * no_of_nodes, h_graph_visited.data, 0, NULL, NULL);
		ExitError(checkErr(err, "Failed to copy data to device!"));
		clEnqueueWriteBuffer(command_queue, d_cost, CL_TRUE, 0, sizeof(int) * no_of_nodes, h_cost.data, 0, NULL, NULL);
		ExitError(checkErr(err, "Failed to copy data to device!"));



		// Set Arguments for kernels 1 and 2
		//
		clSetKernelArg(kernel[0], 0, sizeof(cl_mem), (void*)&d_graph_nodes);
		clSetKernelArg(kernel[0], 1, sizeof(cl_mem), (void*)&d_graph_edges);
		clSetKernelArg(kernel[0], 2, sizeof(cl_mem), (void*)&d_graph_mask);
		clSetKernelArg(kernel[0], 3, sizeof(cl_mem), (void*)&d_updating_graph_mask);
		clSetKernelArg(kernel[0], 4, sizeof(cl_mem), (void*)&d_graph_visited);
		clSetKernelArg(kernel[0], 5, sizeof(cl_mem), (void*)&d_cost);
		clSetKernelArg(kernel[0], 6, sizeof(unsigned int), (void*)&no_of_nodes);

		clSetKernelArg(kernel[1], 0, sizeof(cl_mem), (void*)&d_graph_mask);
		clSetKernelArg(kernel[1], 1, sizeof(cl_mem), (void*)&d_updating_graph_mask);
		clSetKernelArg(kernel[1], 2, sizeof(cl_mem), (void*)&d_graph_visited);
		clSetKernelArg(kernel[1], 3, sizeof(cl_mem), (void*)&d_over);
		clSetKernelArg(kernel[1], 4, sizeof(unsigned int), (void*)&no_of_nodes);



		auto startTime = chrono::high_resolution_clock::now();

		do
		{
			//cout << "Iteration " << k << endl;


			stop = 0;

			// Copy stop to device
			clEnqueueWriteBuffer(command_queue, d_over, CL_TRUE, 0, sizeof(int), (void*)&stop, 0, NULL, NULL);
			ExitError(checkErr(err, "Failed to copy data to device!"));

			// Run kernel[0] and kernel[1]
			cl_int err = clEnqueueNDRangeKernel(
					command_queue, kernel[0], 1, NULL,
					workSize_1, localWorkSize_1, 0, NULL, &clContext.events[0]);
			clFinish(command_queue);
			
			ExitError(checkErr(err, "Failed to execute kernel1!"));

			
			err = clEnqueueNDRangeKernel(command_queue, kernel[1], 1, NULL,
					workSize_2, localWorkSize_2, 0, NULL, &clContext.events[1]);
			clFinish(command_queue);
			
			ExitError(checkErr(err, "Failed to execute kernel2!"));


			// Get kernel execution times
			if(irun != 0)
			{
				for(unsigned int i = 0; i < clContext.events.size(); i++)
				{
					clGetEventProfilingInfo(clContext.events[i], CL_PROFILING_COMMAND_START, sizeof(k_time_start), &k_time_start, NULL);
					clGetEventProfilingInfo(clContext.events[i], CL_PROFILING_COMMAND_END, sizeof(k_time_end), &k_time_end, NULL);
					k_temp = k_time_end - k_time_start;
					kernel_time[i] += (k_temp / 1000000.0);
					//cout << (k_total_time / 1000000.0) << endl;
				}
			}



			// Copy stop from device
			clEnqueueReadBuffer(command_queue, d_over, CL_TRUE, 0, sizeof(int), (void*)&stop, 0, NULL, NULL);
			ExitError(checkErr(err, "Failed to read data from the device!"));
			
			k++;

		}while(stop == 1);

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
	
	cout << "Kernel executed " << k << " times" << endl;
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
	clEnqueueReadBuffer(command_queue, d_cost, CL_TRUE, 0, sizeof(int)*no_of_nodes, (void*)h_cost.data, 0, NULL, NULL);
	ExitError(checkErr(err, "Failed to read data from the device!"));



	// Verification step
	//
	for(int i = 0; i < no_of_nodes; i++)
	{
		h_graph_mask[i]          = 0;
		h_updating_graph_mask[i] = 0;
		h_graph_visited[i]       = 0;
	}
	h_graph_mask[source]    = 1;
	h_graph_visited[source] = 1;

	bfs_cpu(no_of_nodes, h_graph_nodes.data, edge_list_size, h_graph_edges.data, h_graph_mask.data, h_updating_graph_mask.data, h_graph_visited.data, h_cost_ref.data);

	bool passed = std::equal(h_cost.data, h_cost.data+no_of_nodes, h_cost_ref.data);

	cout << "Verification: " << (passed? "Passed": "Failed") << endl;




	// Store the result into a file
	//
	ofstream output_file("bfs_result.txt");
	if(!output_file.is_open()){ cerr << "Unable to write to \"bfs_result.txt\"" << endl; exit(EXIT_FAILURE); }
	for(unsigned int i = 0; i < no_of_nodes; i++)
	{
		output_file << i << ") cost:" << h_cost[i] << "\n";
	}
	cout << "Result stored in bfs_result.txt" << endl;


	// Cleanup memory
	//
	for (unsigned int ki = 0; ki < clContext.kernels.size(); ki++)
   	{
		clReleaseKernel(kernel[ki]);
	}
	clReleaseProgram(clContext.program);
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);
	clReleaseMemObject(d_graph_nodes);
	clReleaseMemObject(d_graph_edges);
	clReleaseMemObject(d_graph_mask);
	clReleaseMemObject(d_updating_graph_mask);
	clReleaseMemObject(d_graph_visited);
	clReleaseMemObject(d_cost);
	clReleaseMemObject(d_over);


	return 0;
}
