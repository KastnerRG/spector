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
 * Filename: sobel_base.cpp
 * Version: 1.0
 * Description: Sobel filter OpenCL benchmark.
 * Author: Pingfan Meng
 */

#define MAX_SOURCE_SIZE 1000000000

#include <stdio.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <string.h>

#include "../../common/include/opencl_utils.h"

#include "timer.h"



using namespace spector;


int main(int argc, char *argv[])
{
	unsigned int * image_in;
	image_in=(unsigned int *)malloc(sizeof(unsigned int)*H*W);
	if (image_in==NULL)
	{
		printf("malloc image_in failed!\n");
		return 1;
	}

	unsigned int * image_out;
	image_out=(unsigned int *)malloc(sizeof(unsigned int)*H*W);
	if (image_out==NULL)
	{
		printf("malloc image_out failed!\n");
		return 1;
	}

	unsigned int * image_out_ref;
	image_out_ref=(unsigned int *)malloc(sizeof(unsigned int)*H*W);
	if (image_out==NULL)
	{
		printf("malloc image_out_ref failed!\n");
		return 1;
	}


	FILE * f_input;
	FILE * f_output;

	f_input=fopen("image_in.txt","r");
	for (int i=0;i<H*W;i++)
	{
		fscanf (f_input, "%u", &image_in[i]);
	}
	fclose(f_input);


	f_input=fopen("image_out_ref.txt","r");
	for (int i=0;i<H*W;i++)
	{
		fscanf (f_input, "%u", &image_out_ref[i]);
	}
	fclose(f_input);




	cl_int error;




	// Initialize OpenCL
	ClContext clContext;

	cl_device_type device_type = CL_DEVICE_TYPE_ACCELERATOR;

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
			printf("Warning! Device not recognized, using FPGA");
		}
	}


	std::vector<std::string> kernel_names;
	kernel_names.push_back("sobel_filter");

	const char* cl_filename   = CL_FILE_NAME;
	const char* aocx_filename = AOCX_FILE_NAME;

	if(!init_opencl(&clContext, kernel_names, device_type, cl_filename, aocx_filename, false)){ exit(EXIT_FAILURE); }

	cl_context context             = clContext.context;
	cl_command_queue command_queue = clContext.queues[0];
	cl_program program             = clContext.program;
	cl_kernel sobel_kernel         = clContext.kernels[0];



	//allocate argument arrays for the kernel
	cl_mem image_in_dev = NULL;
	cl_mem image_out_dev = NULL;


	image_in_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,sizeof(unsigned int)*H*W, NULL, &error);
	if (error != CL_SUCCESS) 
	{
		printf("image_in_dev create failed:%d\n",error);
		return 1;
	}

	image_out_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,sizeof(unsigned int)*H*W, NULL, &error);
	if (error != CL_SUCCESS) 
	{
		printf("image_out_dev create failed:%d\n",error);
		return 1;
	}


	//set arg for the kernel
	error = clSetKernelArg (sobel_kernel,
			0,
			sizeof(cl_mem),
			(void *) &image_in_dev);
	if (error != CL_SUCCESS) 
	{
		printf("Set arg image_in_dev failed:%d\n",error);
		return 1;
	}


	error = clSetKernelArg (sobel_kernel,
			1,
			sizeof(cl_mem),
			(void *) &image_out_dev);
	if (error != CL_SUCCESS) 
	{
		printf("Set arg image_out_dev) failed:%d\n",error);
		return 1;
	}





	//input arrays host -> device
	error=clEnqueueWriteBuffer (command_queue,
			image_in_dev,
			CL_TRUE,
			0,
			sizeof(unsigned int)*H*W,
			image_in,
			0,
			NULL,
			NULL);
	if (error != CL_SUCCESS)
	{
		printf("Write image_in_dev buffer failed:%d\n",error);
		return 1;
	}



	GET_TIME_INIT(2);
	GET_TIME_VAL(0);
	//execute the kernel

	size_t global_work_size[2];

	int GOBALDIM_X=W/SUBDIM_X/SIMD_X;
	int GOBALDIM_Y=H/SUBDIM_Y/SIMD_Y;

	int GRIDDIM_X=GOBALDIM_X/BLOCKDIM_X+(GOBALDIM_X%BLOCKDIM_X?1:0);
	int GRIDDIM_Y=GOBALDIM_Y/BLOCKDIM_Y+(GOBALDIM_Y%BLOCKDIM_Y?1:0);

	global_work_size[0]=GRIDDIM_X*BLOCKDIM_X;
	global_work_size[1]=GRIDDIM_Y*BLOCKDIM_Y;

	size_t local_work_size[2];
	local_work_size[0]=BLOCKDIM_X;
	local_work_size[1]=BLOCKDIM_Y;


	error = clEnqueueNDRangeKernel (command_queue,
			sobel_kernel,
			2,
			NULL,
			global_work_size,
			local_work_size,
			0, NULL, NULL);

	if (error != CL_SUCCESS) 
	{
		printf("Execute task kernel failed:%d\n",error);
		return 1;
	}


	clFinish(command_queue);

	GET_TIME_VAL(1);
	//results device -> host
	error = clEnqueueReadBuffer(command_queue, image_out_dev, CL_TRUE, 0,
			sizeof(unsigned int)*H*W,image_out, 0, NULL, NULL);
	if (error != CL_SUCCESS) 
	{
		printf("image_out Device -> host failed:%d\n",error);
		return 1;
	}


	printf("Run-time is:%f ms \n",ELAPSED_TIME_MS(1, 0));
	print_rsl;

	error = clReleaseMemObject(image_in_dev);
	if (error != CL_SUCCESS) 
	{
		printf("Release image_in_dev failed:%d\n",error);
		return 1;
	}
	error = clReleaseMemObject(image_out_dev);
	if (error != CL_SUCCESS) 
	{
		printf("Release image_out_dev failed:%d\n",error);
		return 1;
	}


	int precision_flag=0;

	f_output=fopen("image_out.txt","w");
	for (int i=0;i<H;i++)
	{
		for (int j=0;j<W;j++)
		{
			fprintf(f_output,"%u ",image_out[i*W+j]);
			if(i!=0&&j!=0&&i!=H-1&&j!=W-1)
				precision_flag+=(image_out[i*W+j]-image_out_ref[i*W+j])*(image_out[i*W+j]-image_out_ref[i*W+j]);
		}
		fprintf(f_output,"\n");
	}
	fclose(f_output);



	if (precision_flag)
		printf("Failed:Output does not match the golden out!!!!\n error is:%d\n",precision_flag);
	else
		printf("Passed:Output matches the golden out!!!!\n");

	free(image_in);
	free(image_out);
	free(image_out_ref);

}
