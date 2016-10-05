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
 * Filename: spmv_host_base.cpp
 * Version: 1.0
 * Description: Sparse matrix-vector multiplication OpenCL benchmark.
 * Author: Pingfan Meng
 */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <assert.h>
#include <errno.h>
#include "CL/opencl.h"
#include "my_spmv_common.h"

#include "timer.h"


#define MAX_SOURCE_SIZE 10000000

#define NUM_REP 1000


int main(int argc, char** argv)
{
	int result_error=0;
	FILE * fp=NULL;
	 
	unsigned int num_matrices;

	if(argc < 2)
	{
		printf("Usage: <filename>\n");
		exit(1);
	} 
	printf("Reading csr File\n");

	csr_matrix * csr = read_csr(&num_matrices,argv[1]);

	int num_rows=csr[0].num_rows;
	int num_cols=csr[0].num_cols;

	//read csr
	//read x
	float * x_host=(float *)malloc(sizeof(float)*num_cols);
	
	for(unsigned int i = 0; i < num_cols; i++)
	{
		x_host[i] = ((float)i)*0.035;
	}

	
	//read y
	float * y_host=(float *)malloc(sizeof(float)*num_rows);

	for(unsigned int i = 0; i < num_rows; i++)
	{
		y_host[i] = ((float)i)*0.016;
	}

	
	///init cl
	cl_int error;
   	cl_platform_id platform_id;
   	cl_device_id device_id;

	cl_context context;
   	cl_command_queue command_queue;
	cl_program program;
	cl_kernel kernel;

   	platform_id = NULL;
   	device_id = NULL;

   	cl_uint ret_num_devices;
   	cl_uint ret_num_platforms;

  	context = NULL;
   	command_queue = NULL;
   	program = NULL;

   	
   	kernel = NULL;
   	
   

   	GET_TIME_INIT(2);
   	char fileName[] = CL_FILE_NAME;
   	char *source_str;
   	size_t source_size;



   
   	//get platform IDs
   	error = clGetPlatformIDs(1, &platform_id, &ret_num_platforms);
   	if (error != CL_SUCCESS) 
   	{
		printf("Get Platform IDs failed:%d\n",error);
        	return 1;
   	}

   	//get device IDs
   	error = clGetDeviceIDs( platform_id, CL_DEVICE_TYPE_DEFAULT, 1, &device_id, &ret_num_devices);
   	if (error != CL_SUCCESS) 
   	{
		printf("Get Device IDs failed:%d\n",error);
        	return 1;
   	}

   	//get the device name
   	char device_name[1024];
   	error=clGetDeviceInfo(device_id,CL_DEVICE_NAME,1024,device_name,NULL);

   	if (error != CL_SUCCESS) 
   	{
		printf("Get Device Info failed:%d\n",error);
        	return 1;
   	}
   	printf("The device used is:%s\n",device_name);

   	//create context
   	context = clCreateContext( NULL, 1, &device_id, NULL, NULL, &error);
   	if (error != CL_SUCCESS) 
   	{
		printf("Create context failed:%d\n",error);
        	return 1;
   	}

   	//create command queue
   	command_queue = clCreateCommandQueue(context, device_id, 0, &error);
   	if (error != CL_SUCCESS) 
   	{
		printf("Create command queue failed:%d\n",error);
        	return 1;
   	}


	char file_name[] = CL_FILE_NAME;
  	 //printf("Checkpoint before seek \n");
   	fp = fopen(file_name, "r");
   	fseek(fp, 0, SEEK_END);
   	//printf("Checkpoint after seek \n");
   
   	size_t binary_length = ftell(fp);
   	const unsigned char *binary = (unsigned char*) malloc(sizeof(unsigned char) * binary_length);
   	assert(binary && "Malloc failed");
   	//printf("Checkpoint after malloc binary\n");
   	rewind(fp);
   	//printf("Checkpoint before fread the binary\n");
   	if (fread((void *)binary, binary_length, 1, fp) == 0)
   	{
     		printf("Failed to read .aocx.\n");
     		return -10000;
   	}
  	fclose(fp);

   	//create program
	cl_int errnum, status;
   	program = clCreateProgramWithBinary(context, 1, &device_id, &binary_length, (const unsigned char **)&binary, &status, &errnum);

   	if (error != CL_SUCCESS) 
   	{
		printf("Create program failed:%d\n",error);
        	return 1;
   	}


   	error = clBuildProgram (program,
  		0,
  		NULL,
  		NULL,
  		NULL,
  		NULL);
   	if (error != CL_SUCCESS) 
   	{
		printf("Build program failed:%d\n",error);
        	return 1;
   	}

	kernel = clCreateKernel(program, "csr", &error);
   	if (error != CL_SUCCESS) 
   	{
		printf("Create kernel %d failed:%d\n",1,error);
        	return 1;
   	}

	//cl memory
	cl_mem * csr_ap = (cl_mem*) malloc(sizeof(cl_mem)*num_matrices);
	cl_mem * csr_aj = (cl_mem*) malloc(sizeof(cl_mem)*num_matrices);
	cl_mem * csr_ax = (cl_mem*) malloc(sizeof(cl_mem)*num_matrices);
	cl_mem * x_loc = (cl_mem*) malloc(sizeof(cl_mem)*num_matrices);
	cl_mem * y_loc = (cl_mem*) malloc(sizeof(cl_mem)*num_matrices);


	for(int k=0; k<num_matrices; k++)
	{
		csr_ap[k]=clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)*(csr[k].num_rows+1), NULL, &error);
		csr_aj[k]=clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)*csr[k].num_nonzeros, NULL, &error);
		csr_ax[k]=clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(float)*csr[k].num_nonzeros, NULL, &error);
		x_loc[k]=clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)*csr[k].num_cols, NULL, &error);
		y_loc[k]=clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int)*csr[k].num_rows, NULL, &error);
	}
	
	float run_time_total=0;

	for(int k=0; k<num_matrices; k++)
	{
		
		error=clEnqueueWriteBuffer(command_queue, csr_ap[k], CL_TRUE, 0, sizeof(int)*(csr[k].num_rows+1), csr[k].Ap, 0, NULL, NULL);
		if (error != CL_SUCCESS) 
   		{
			printf("Copy Ap to device error:%d\n",error);
        		return 1;
   		}
		error=clEnqueueWriteBuffer(command_queue, csr_aj[k], CL_TRUE, 0, sizeof(int)*csr[k].num_nonzeros, csr[k].Aj, 0, NULL, NULL);
		if (error != CL_SUCCESS) 
   		{
			printf("Copy Aj to device error:%d\n",error);
        		return 1;
   		}
		error=clEnqueueWriteBuffer(command_queue, csr_ax[k], CL_TRUE, 0, sizeof(float)*csr[k].num_nonzeros, csr[k].Ax, 0, NULL, NULL);
		if (error != CL_SUCCESS) 
   		{
			printf("Copy Ax to device error:%d\n",error);
        		return 1;
   		}
		error=clEnqueueWriteBuffer(command_queue, x_loc[k], CL_TRUE, 0, sizeof(float)*csr[k].num_cols, x_host, 0, NULL, NULL);
		if (error != CL_SUCCESS) 
   		{
			printf("Copy X to device error:%d\n",error);
        		return 1;
   		}
		error=clEnqueueWriteBuffer(command_queue, y_loc[k], CL_TRUE, 0, sizeof(float)*csr[k].num_rows, y_host, 0, NULL, NULL);
		if (error != CL_SUCCESS) 
   		{
			printf("Copy Y to device error:%d\n",error);
        		return 1;
   		}
		clFinish(command_queue);



		error = clSetKernelArg(kernel, 0, sizeof(int), &csr[k].num_rows);
		error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &csr_ap[k]);
		error |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &csr_aj[k]);
		error |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &csr_ax[k]);
		error |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &x_loc[k]);
		error |= clSetKernelArg(kernel, 5, sizeof(cl_mem), &y_loc[k]);
		if (error != CL_SUCCESS) 
   		{
			printf("Failed to set kernel arguments:%d\n",error);
        		return 1;
   		}

		size_t global_work_size[2];
   		global_work_size[0]=num_rows;
   		global_work_size[1]=1;

   		size_t local_work_size[2];
   		local_work_size[0]=BLOCKDIM;
   		local_work_size[1]=1;

		float* device_out=(float*)malloc(sizeof(float)*csr[k].num_rows);
		float * host_out = (float*)malloc(sizeof(float)*csr[k].num_rows);

		for(int i=0; i<NUM_REP; i++) 
		{
			

			
   			GET_TIME_VAL(0);
			error = clEnqueueNDRangeKernel(command_queue, kernel, 1, NULL, global_work_size, local_work_size, 0, NULL, NULL);
			if (error != CL_SUCCESS) 
   			{
				printf("Kernel execution error:%d\n",error);
        			return 1;
   			}
			clFinish(command_queue);
			GET_TIME_VAL(1);
			run_time_total+=ELAPSED_TIME_MS(1, 0);
			
			/* Read back the results from the device to verify the output */
			error = clEnqueueReadBuffer(command_queue, y_loc[k], CL_TRUE, 0, sizeof(float)*csr[k].num_rows, device_out, 0, NULL, NULL);
			clFinish(command_queue);
		

			for(k=0; k<num_matrices; k++)
			{

				spmv_csr_cpu(&csr[k],x_host,y_host,host_out);

				result_error|=float_array_comp(host_out,device_out,csr[k].num_rows,i+1);
			}

		}

		

		free(device_out);
		free(host_out);					
		

		
	}

	//destroy opencl
	clReleaseCommandQueue(command_queue);
	clReleaseKernel(kernel);


	///free all the device memory
	for(int k=0; k<num_matrices; k++)
	{
		error = clReleaseMemObject(csr_ap[k]);

		error |= clReleaseMemObject(csr_aj[k]);

		error |= clReleaseMemObject(csr_ax[k]);

		error |= clReleaseMemObject(x_loc[k]);
	
		error |= clReleaseMemObject(y_loc[k]);

		if (error != CL_SUCCESS) 
   		{
			printf("Free device mem error:%d\n",error);
        		return 1;
   		}
		
	}


	///free all the host memory
	free_csr(csr,num_matrices);
	free(x_host);
	free(y_host);

	if (result_error)
	{
		printf("Something wrong with the device result\n");
	}	
	else
	{
		printf("Pass: Device result matches CPU result\n");
		printf("Run-time is:%f ms \n",run_time_total/NUM_REP);
		print_rsl;
	}
	return 0;
}

