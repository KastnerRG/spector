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
 * Filename: mergesort.h
 * Version: 1.0
 * Description: Merge sort OpenCL benchmark.
 * Author: Quentin Gautier
 */


#ifndef MERGESORT_H_
#define MERGESORT_H_


#include <vector>

#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#include <CL/cl.h>


//---------------------------------------------
// Pre-processor macros
//---------------------------------------------

#define ReturnError(x) if(!(x)){ return false; }
#define ExitError(x)   if(!(x)){ exit(1); }
#define checkErr(x, y) checkErr_(x, y, __LINE__, __FILE__)



//---------------------------------------------
// Type definitions
//---------------------------------------------

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



//---------------------------------------------
// Functions
//---------------------------------------------


inline bool checkErr_(cl_int err, const char * name, int line = -1, const char* file = NULL)
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


#endif // MERGESORT_H_
