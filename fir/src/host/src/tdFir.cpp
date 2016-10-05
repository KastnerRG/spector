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
 * Filename: tdFir.cpp
 * Version: 1.0
 * Description: FIR filter OpenCL benchmark.
 * Author (modifications): Quentin Gautier
 * Note: This work is based on the Altera OpenCL FIR filter example.
 *       See below for original authors and copyright.
 */

#include "tdFir.h"
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include "AOCLUtils/aocl_utils.h"
#include "parameters.h"

using namespace aocl_utils;
using namespace std;

#ifndef RUN_ON_DEVICE
#define RUN_ON_DEVICE 1
#endif

#define STRING_BUFFER_LEN 1024

#ifndef CL_MEM_BANK_1_ALTERA
#define CL_MEM_BANK_1_ALTERA 0
#endif
#ifndef CL_MEM_BANK_2_ALTERA
#define CL_MEM_BANK_2_ALTERA 0
#endif

// ACL runtime configuration
static cl_platform_id platform = NULL;
static cl_device_id device     = NULL;
static cl_context context      = NULL;
static cl_command_queue queue  = NULL;
static cl_kernel kernel        = NULL;
static cl_program program      = NULL;
//static cl_int status = 0;
cl_mem dev_datainput;
cl_mem dev_filterconst;
cl_mem dev_result;
cl_event myEvent;

// Helper Function prototypes
bool initOpenCL(cl_device_type);
void cleanup();


int main(int argc, char **argv)
{
  struct tdFirVariables tdFirVars;

  tdFirVars.arguments = 1;
  tdFirVars.dataSet = 4;

  if(!setCwdToExeDir()) {
    return -1;
  }
  
  /*
    I need to declare some variables:
      -pointers to data, filter, and result
      -inputLength;
    These are all in the tdFirVariables structure in tdFir.h
    Along with the structure definition, I also instantiate an 
    instanced called tdFirVars.  This is the same as doing:
    
    struct tdFirVaribles tdFirVars;
  */
  
  /*
    In tdFirSetup, I want to perform tasks that I do NOT want to include
    in my timing.  For example:
      -allocating space for data, filter, and result
	  The declarations for this function can be found in tdFir.h, while
    the definitions of these functions can be found in tdFir.c.
  */
  tdFirSetup(&tdFirVars);


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
		std::cerr << "Warning! Device not recognized, using FPGA" << std::endl;
    }
  }

  int num_runs = 1;
  if(argc >= 3)
  {
    num_runs = atoi(argv[2]);
  }


  if (RUN_ON_DEVICE)
  {
    // Perform FIR computation on one of FPGA/GPU/CPU device
    if(!initOpenCL(device_type)) {
      return -1;
    }

	// Run for warm-up
    tdFirFPGA(&tdFirVars);

	double totalTime = 0.0;
	for(int i = 0; i < num_runs; i++)
	{
      tdFirFPGA(&tdFirVars);
      totalTime += tdFirVars.time.data[0];
	}
	std::cout << "Total time: " << (totalTime/num_runs)*1000.0 << " ms" << std::endl;
  }
  else
  {
    // Perform FIR computation on CPU
    tdFirCPU(&tdFirVars);
  }

  /*
    In tdFirComplete(), I first want to output my result to output.dat.  
    I then want to do any required clean up.
  */
  tdFirComplete(&tdFirVars);

  /*
    Run the verification routine to ensure that our results were correct.
  */
  tdFirVerify(&tdFirVars);
  tdFirVerifyComplete(&tdFirVars);
  
  return 0;
}


/*
  In tdFirSetup, I want to perform tasks that I do NOT want to include
  in my timing.  For example:
    -allocating space for data, filter, result
    -read in inputs and initalize the result space to 0.
  The declarations for this function can be found in tdFir.h, while
  the definitions of these functions can be found in tdFir.c.
*/
void tdFirSetup(struct tdFirVariables *tdFirVars)
{
  int inputLength, filterLength, resultLength;
  char dataSetString[100];
  char filterSetString[100];

  sprintf(  dataSetString,"./%d-tdFir-input.dat" ,tdFirVars->dataSet);
  sprintf(filterSetString,"./%d-tdFir-filter.dat",tdFirVars->dataSet);

  /*
    input read from file 'input.dat', and stored at:   tdFirVars->input.data
    fileter read from file 'filter.dat' and stored at: tdFirVars->filter.data
  */
  readFromFile(float, dataSetString, tdFirVars->input);    
  readFromFile(float, filterSetString, tdFirVars->filter);  

  pca_create_carray_1d(float, tdFirVars->time, 1, PCA_REAL);
  
  inputLength            = tdFirVars->input.size[1];
  filterLength           = tdFirVars->filter.size[1];
  resultLength           = inputLength + filterLength - 1;
  tdFirVars->numFilters   = tdFirVars->filter.size[0];
  tdFirVars->inputLength  = tdFirVars->input.size[1];
  tdFirVars->filterLength = tdFirVars->filter.size[1];
  tdFirVars->resultLength = resultLength;
  tdFirVars->time.data[0] = 0.0f;
  tdFirVars->time.data[1] = 0.0f;
  tdFirVars->time.data[2] = 0.0f;
  
  if(tdFirVars->numFilters % TOTAL_WORK_ITEMS != 0 || tdFirVars->numFilters < TOTAL_WORK_ITEMS)
  {
	  std::cerr << "Error: The number of work-items must match the number of filters!" << std::endl;
	  exit(EXIT_FAILURE);
  }

  pca_create_carray_2d(float, tdFirVars->result, tdFirVars->numFilters, resultLength, PCA_COMPLEX);
  /*
    Make sure that the result starts out as 0.
  */
  zeroData(tdFirVars->result.data, resultLength, tdFirVars->numFilters);
}

/*
 * This is the original Time Domain FIR Filter implementation to be  
 * executed on CPU
 */
void tdFirCPU(struct tdFirVariables *tdFirVars)
{
  int index;
  int filter;
  float * inputPtr  = tdFirVars->input.data;
  float * filterPtr = tdFirVars->filter.data;
  float * resultPtr = tdFirVars->result.data;
  float * inputPtrSave  = tdFirVars->input.data;
  float * filterPtrSave = tdFirVars->filter.data;
  float * resultPtrSave = tdFirVars->result.data;
  int  filterLength = tdFirVars->filterLength;
  int  inputLength  = tdFirVars->inputLength;  
  int  resultLength = filterLength + inputLength - 1;
  double startTime = getCurrentTimestamp();
  double stopTime = 0.0f;

  for(filter = 0; filter < tdFirVars->numFilters; filter++)
  {
    inputPtr  = inputPtrSave  + (filter * (2*inputLength)); 
    filterPtr = filterPtrSave + (filter * (2*filterLength)); 
    resultPtr = resultPtrSave + (filter * (2*resultLength)); 

    /*
    	elCplxMul does an element wise multiply of the current filter element by
    	the entire input vector.
    	Input Parameters:
    	tdFirVars->input.data  - pointer to input
    	tdFirVars->filter.data - pointer to filter
    	tdFirVars->result.data - pointer to result space
    	tdFirVars->inputLength - integer value representing length of input
     */
    for(index = 0; index < filterLength; index++)
	  {
  	  elCplxMul(inputPtr, filterPtr, resultPtr, tdFirVars->inputLength);
  	  resultPtr+=2;
  	  filterPtr+=2;
  	}/* end for filterLength*/
  }/* end for each filter */

  /*
    Stop the timer.  Print out the
    total time in Seconds it took to do the TDFIR.
  */
  stopTime = getCurrentTimestamp();
  tdFirVars->time.data[0] = stopTime - startTime;

  printf("Done.\n  Latency: %f s.\n", tdFirVars->time.data[0]);
  //printf("  Throughput: %.3f GFLOPs.\n", 268.44f / tdFirVars->time.data[0] / 1000.0f );
}

/*
  elCplxMul does an element wise multiply of the current filter element by
  the entire input vector.
 */
void elCplxMul(float *dataPtr, float *filterPtr, float *resultPtr, int inputLength)
{
  int index;
  float filterReal = *filterPtr; 
  float filterImag = *(filterPtr+1);

  for(index = 0; index < inputLength; index++)
  {
    /*      COMPLEX MULTIPLY   */
    /* real  */
    *resultPtr += (*dataPtr) * filterReal - (*(dataPtr+1)) * filterImag;
    resultPtr++;
    /* imag  */
    *resultPtr += (*dataPtr) * filterImag + (*(dataPtr+1)) * filterReal;
    resultPtr++;
    dataPtr+=2;
  }
}

/* 
  This routine sets up the TDFIR kernel parameters and runs it on the FPGA

  The filterLength here is obtained from the data file, and should be 
  kept consistent with what the kernel is compiled with.  
  By default, the kernel is compiled with 128 taps.
  
  To improve the efficiency of the kernel, we insert padding to the input data and the
  result data arrays.  Thus, we first allocate a larger data array
  
 */
void tdFirFPGA(struct tdFirVariables *tdFirVars)
{
  int err;
  double startTime, stopTime;
  
  // These are the pointers to original input data and filter constants provided
  float * inputPtr  = tdFirVars->input.data;
  float * filterPtr = tdFirVars->filter.data;
  // This is the pointer of the computed result 
  float * resultPtr = tdFirVars->result.data;
  
  int  filterLength = tdFirVars->filterLength;
  int  inputLength  = tdFirVars->inputLength;  
  int  resultLength = filterLength + inputLength - 1;
  
  // These are pointers to the padded input and result data.  
  // We will copy data to and from the original arrays.
  float * paddedInputPtr = 0;
  float * paddedResultPtr = 0;
  
  unsigned paddingEndTotalLength = ceil((filterLength - 1 + inputLength) / float(KNOB_NUM_PARALLEL)) * KNOB_NUM_PARALLEL - inputLength;
  unsigned extraPadding = paddingEndTotalLength - (filterLength - 1);

  unsigned paddingCoefLoad = NUM_COEF_LOADS * KNOB_NUM_PARALLEL;


  // padded number of points per filter, 16 points of zero when we are loading 
  // the filter coefficients and 127 points of zero at the end when we're 
  // computing the tail end for the result
  unsigned paddedNumInputPoints = inputLength + paddingCoefLoad + paddingEndTotalLength;
  //each point is a complex, so need to multiply be 2 (for real, imag)
  unsigned paddedSingleInputLength = 2 * paddedNumInputPoints;
  //this is the size of the input data buffers we need to allocate
  unsigned totalDataInputLength = paddedSingleInputLength * tdFirVars->numFilters;
  //this is the number of total points we are computing
  unsigned totalDataInputLengthKernelArg = (paddedNumInputPoints * (tdFirVars->numFilters / TOTAL_WORK_ITEMS));
  unsigned numIterationsKernelArg = totalDataInputLengthKernelArg / KNOB_NUM_PARALLEL;
  // these are just 1 less than the padded single input points, used as a parameter
  // for the kernel to know when to start loading in the next set of filter 
  // coefficients
  unsigned paddedSingleInputLengthMinus1KernelArg = paddedNumInputPoints - KNOB_NUM_PARALLEL;

  unsigned totalFiltersLengthKernelArg = filterLength * (tdFirVars->numFilters / TOTAL_WORK_ITEMS);

  std::cout << paddingCoefLoad << "  " << inputLength << "  " << paddingEndTotalLength << std::endl;

  // we pad the beginning of the result buffer with 16 complex points of zero
  unsigned paddedNumResultLength = 2*(resultLength+extraPadding+paddingCoefLoad);

  startTime = getCurrentTimestamp();

  // this assumes that the inputLength is the same for each filter
  dev_datainput = clCreateBuffer(context, CL_MEM_READ_ONLY, 
                        sizeof(float)*totalDataInputLength, NULL, &err);
  checkError(err, "Failed to allocate device memory!");
  dev_filterconst = clCreateBuffer(context, 
                        CL_MEM_READ_ONLY | CL_MEM_BANK_2_ALTERA, 
                        sizeof(float)* 2 * filterLength * tdFirVars->numFilters,
                        NULL, &err);
  checkError(err, "Failed to allocate device memory!");
  dev_result = clCreateBuffer(context, CL_MEM_WRITE_ONLY | CL_MEM_BANK_2_ALTERA, 
                        sizeof(float) * paddedNumResultLength * tdFirVars->numFilters,
                        NULL, &err);
  checkError(err, "Failed to allocate device memory!");
    
  paddedInputPtr = (float* ) alignedMalloc(sizeof(float) * totalDataInputLength);
  memset(paddedInputPtr, '\0', sizeof(float) * totalDataInputLength);

  // Need to copy the input data into the padded structure 
  // (with an offset of 16 complex points)
  for (int filter = 0; filter < tdFirVars->numFilters; filter++)
  {
    memcpy(paddedInputPtr + filter * paddedSingleInputLength + 2*paddingCoefLoad,
           inputPtr + (filter * (2*inputLength)), 
           sizeof(float) * 2 * inputLength);
  }

  paddedResultPtr = (float* ) alignedMalloc(
					sizeof(float) * paddedNumResultLength * tdFirVars->numFilters);
  memset(paddedResultPtr, '\0', 
		 sizeof(float) * paddedNumResultLength * tdFirVars->numFilters);
  
  err = clEnqueueWriteBuffer(queue, dev_datainput, CL_TRUE, 0, 
                     sizeof(float)*totalDataInputLength, 
                     paddedInputPtr, 0, NULL, &myEvent);
  checkError(err, "Failed to write input buffer!");
  err = clEnqueueWriteBuffer(queue, dev_filterconst, CL_TRUE, 0, 
                     sizeof(float) * 2 * filterLength * tdFirVars->numFilters,
                     filterPtr, 0, NULL, &myEvent);
  checkError(err, "Failed to write filterconst!");
  clFinish(queue);

  printf("tdFirVars: inputLength = %d, resultLength = %d, filterLen = %d, numFilters = %d\n",
         inputLength, resultLength, filterLength, tdFirVars->numFilters);

  err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &dev_datainput);
  err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dev_filterconst);
  err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &dev_result);
  err |= clSetKernelArg(kernel, 3, sizeof(unsigned int), 
                        &totalDataInputLengthKernelArg);
  err |= clSetKernelArg(kernel, 4, sizeof(unsigned int), 
                        &numIterationsKernelArg);
  err |= clSetKernelArg(kernel, 5, sizeof(unsigned int),
                        &paddedSingleInputLengthMinus1KernelArg);
  err |= clSetKernelArg(kernel, 6, sizeof(unsigned int),
                        &totalFiltersLengthKernelArg);
  checkError(err, "Failed to set compute kernel arguments!");

  size_t global_work = TOTAL_WORK_ITEMS;
  size_t local_work  = KNOB_NUM_WORK_ITEMS;

  stopTime = getCurrentTimestamp();
  tdFirVars->time.data[1] += (float)(stopTime - startTime);
    
  startTime = getCurrentTimestamp();

  // This runs the actual TDFIR implementation on FPGA
  err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, 
                               &global_work, &local_work, 0, NULL, &myEvent);
  clFinish(queue);
  stopTime = getCurrentTimestamp();
  checkError(err, "Failed to execute tdfir kernel!");
 

  
  cl_ulong k_time_start, k_time_end;
  clGetEventProfilingInfo(myEvent, CL_PROFILING_COMMAND_START, sizeof(k_time_start), &k_time_start, NULL);
  clGetEventProfilingInfo(myEvent, CL_PROFILING_COMMAND_END,   sizeof(k_time_end),   &k_time_end, NULL);
  cl_ulong k_total = k_time_end - k_time_start;


  //tdFirVars->time.data[0] = (float)(stopTime - startTime);
  tdFirVars->time.data[0] = (float)(k_total / 1000000000.0);


  startTime = getCurrentTimestamp();
  err = clEnqueueReadBuffer(queue, dev_result, CL_TRUE, 0, 
                sizeof(float) * paddedNumResultLength * tdFirVars->numFilters,
                paddedResultPtr, 0, NULL, &myEvent);
  checkError(err, "Failed to read result array!");
  clFinish(queue);



  // Copy the result into the original result array, by starting 
  // at an offset of 16 complex points
  for (int filter = 0; filter < tdFirVars->numFilters; filter++)
  {
	  memcpy(resultPtr + 2*filter*resultLength, 
	         paddedResultPtr + filter * paddedNumResultLength + 2*paddingCoefLoad,
	         sizeof(float) * 2 * resultLength);
  }

  stopTime = getCurrentTimestamp();
  tdFirVars->time.data[1] += (float)(stopTime - startTime);
  
  // Print out the total time and throughput for the TDFIR computation
  printf("Done.\n  Latency: %f s.\n", tdFirVars->time.data[0]);
  printf("  Buffer Setup Time: %f s.\n", tdFirVars->time.data[1]);
  //printf("  Throughput: %.3f GFLOPs.\n", 0.26844f / tdFirVars->time.data[0] ); // throughput computation needs to be updated with code changes
}


/////// HELPER FUNCTIONS ///////

bool initOpenCL(cl_device_type device_type = CL_DEVICE_TYPE_ALL) {
  cl_int status;

  int err;



  vector<cl_platform_id> platform_ids;
  vector<cl_device_id> device_ids;
  
  
  // Get platform and devices
  //
  cl_uint num_platforms;
  err = clGetPlatformIDs(0, NULL, &num_platforms);
  checkError(err, "Failed to get number of platforms!");
  
  cout << num_platforms << " platforms:" << endl;
  
  platform_ids.resize(num_platforms);
  
  err = clGetPlatformIDs(num_platforms, platform_ids.data(), NULL);
  checkError(err, "Failed to get platform ID!");
  
  for(cl_uint plat = 0; plat < num_platforms; plat++)
  {
  	size_t sz;
  	err = clGetPlatformInfo(platform_ids[plat], CL_PLATFORM_NAME, 0, NULL, &sz);
  	checkError(err, "Failed to get size of platform name!");
  
  	char* name = new char[sz];
  	err = clGetPlatformInfo(platform_ids[plat], CL_PLATFORM_NAME, sz, name, NULL);
  	checkError(err, "Failed to get platform name!");
  
  	cout << "  - " << name << endl;
  
  
  	cl_uint num_devices;
  	clGetDeviceIDs(platform_ids[plat], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);
  	checkError(err, "Failed to get number of devices!");
  
  	cout << "    with " << num_devices << " device(s):" << endl;
  
  	device_ids.resize(num_devices);
  	err = clGetDeviceIDs(platform_ids[plat], CL_DEVICE_TYPE_ALL, num_devices, device_ids.data(), NULL);
  	checkError(err, "Failed to get devices!");
  
  	for(cl_uint i = 0; i < num_devices; i++)
  	{
  		size_t sz;
  		clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, 0, NULL, &sz);
  		checkError(err, "Failed to get size of device name!");
  
  		char* name = new char[sz];
  		clGetDeviceInfo(device_ids[i], CL_DEVICE_NAME, sz, name, NULL);
  		checkError(err, "Failed to get device name!");
  
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
  checkError(err, "Failed to find a device!");
  
  {
  	size_t sz;
  	clGetDeviceInfo(device_ids[0], CL_DEVICE_NAME, 0, NULL, &sz);
  	checkError(err, "Failed to get size of device name!");
  
  	char* name = new char[sz];
  	clGetDeviceInfo(device_ids[0], CL_DEVICE_NAME, sz, name, NULL);
  	checkError(err, "Failed to get device name!");
  
  	cout << "Using " << name << endl;
  
  	delete[] name;
  }

  device = device_ids[0];




  //// Get the OpenCL platform.
  //platform = findPlatform(""); //Altera");
  //if(platform == NULL) {
  //  printf("ERROR: Unable to find OpenCL platform.\n");
  //  return false;
  //}

  //// User-visible output - Platform information
  //{
  //  char char_buffer[STRING_BUFFER_LEN]; 
  //  printf("Querying platform for info:\n");
  //  printf("==========================\n");
  //  clGetPlatformInfo(platform, CL_PLATFORM_NAME, STRING_BUFFER_LEN, char_buffer, NULL);
  //  printf("%-40s = %s\n", "CL_PLATFORM_NAME", char_buffer);
  //  clGetPlatformInfo(platform, CL_PLATFORM_VENDOR, STRING_BUFFER_LEN, char_buffer, NULL);
  //  printf("%-40s = %s\n", "CL_PLATFORM_VENDOR ", char_buffer);
  //  clGetPlatformInfo(platform, CL_PLATFORM_VERSION, STRING_BUFFER_LEN, char_buffer, NULL);
  //  printf("%-40s = %s\n\n", "CL_PLATFORM_VERSION ", char_buffer);
  //}

  //// Query the available OpenCL devices.
  //scoped_array<cl_device_id> devices;
  //cl_uint num_devices;

  //devices.reset(getDevices(platform, device_type, &num_devices));

  //// We'll just use the first device.
  //device = devices[0];

  //printf("Using Device: %s\n", getDeviceName(device).c_str());








  // Create the context.
  context = clCreateContext(NULL, 1, &device, &oclContextCallback, NULL, &status);
  checkError(status, "Failed to create context");

  // Create the command queue.
  queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &status);
  checkError(status, "Failed to create command queue");

  // Create the program.
  std::string binary_file = getBoardBinaryFile("tdfir", device);
  if(device_type == CL_DEVICE_TYPE_ACCELERATOR)
  {
	  printf("Using AOCX: %s\n", binary_file.c_str());
	  program = createProgramFromBinary(context, binary_file.c_str(), &device, 1);
  }
  else
  {
	  std::string opencl_file = getOpenCLFile("../device/tdfir");
	  printf("Using CL file: %s\n", opencl_file.c_str());
	  program = createProgramFromCL(context, opencl_file.c_str());
  }


  // Build the program that was just created.
  status = clBuildProgram(program, 0, NULL, "-I../host/inc/", NULL, NULL);


  {
	  char buildLog[1 << 16];
	  cl_int status2 = clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, sizeof(buildLog), buildLog, NULL);
	  checkError(status2, "Failed to build program");
	  printf("----------------------\n%s\n----------------------\n", buildLog);
  }
  checkError(status, "Failed to build program");


  // Create the kernel - name passed in here must match kernel name in the
  // original CL file, that was compiled into an AOCX file using the AOC tool
  const char *kernel_name = "tdfir";  // Kernel name, as defined in the CL file
  kernel = clCreateKernel(program, kernel_name, &status);
  checkError(status, "Failed to create kernel");

  return true;
}

// Free the resources allocated during initialization
void cleanup() {
  if(kernel) 
    clReleaseKernel(kernel);  
  if(program) 
    clReleaseProgram(program);
  if(queue) 
    clReleaseCommandQueue(queue);
  if(context) 
    clReleaseContext(context);
}

void printVector(float * dataPtr, int inputLength)
{
  int index;
  printf("Start of Vector: \n");
  for(index = 0; index < inputLength; index++)
  {
    printf("(%f,%fi) \n",*dataPtr, *(dataPtr+1));
    dataPtr = dataPtr + 2;
  }
  printf("End of Vector. \n");
}

void zeroData(float *dataPtr, int length, int filters)
{
  int index, filter;

  for(filter = 0; filter < filters; filter++)
  {
    for(index = 0; index < length; index++)
    {
      *dataPtr = 0;
      dataPtr++;
      *dataPtr = 0;
      dataPtr++;
    }
  }
}

/*
  In tdFirComplete, I want to do any required clean up.
    -...
*/
void tdFirComplete(struct tdFirVariables *tdFirVars)
{
  char timeString[100];
  char outputString[100];
  sprintf(timeString,"./%d-tdFir-time.dat",tdFirVars->dataSet);
  sprintf(outputString,"./%d-tdFir-output.dat",tdFirVars->dataSet);
  
  writeToFile(float, outputString, tdFirVars->result);
  writeToFile(float, timeString, tdFirVars->time);
  
  clean_mem(float, tdFirVars->input);
  clean_mem(float, tdFirVars->filter);
  clean_mem(float, tdFirVars->result);
  clean_mem(float, tdFirVars->time);
}

/* ----------------------------------------------------------------------------
Copyright (c) 2006, Massachusetts Institute of Technology
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are  
met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the Massachusetts Institute of Technology nor  
       the names of its contributors may be used to endorse or promote 
       products derived from this software without specific prior written 
       permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF  
THE POSSIBILITY OF SUCH DAMAGE.
---------------------------------------------------------------------------- */

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

/******************************************************************************
** File: tdFir.c
**
** HPEC Challenge Benchmark Suite
** TDFIR Kernel Benchmark
**
** Contents: This file provides definitions for various functions in support 
**           of the generic C TDFIR implementation.                           
**            Inputs: ./<dataset>-tdFir-input.dat          
**                    ./<dataset>-tdFir-filter.dat         
**            Outputs:./<dataset>-tdFir-output.dat         
**                    ./<dataset>-tdFir-time.dat           
**
** Author: Matthew A. Alexander 
**         MIT Lincoln Laboratory
**
******************************************************************************/

