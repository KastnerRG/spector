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
 * Filename: mm_base.cpp
 * Version: 1.0
 * Description: Matrix multiplication OpenCL benchmark.
 * Author: Pingfan Meng
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CL/opencl.h"
#include <assert.h>

#include "timer.h"


#define MAX_SOURCE_SIZE 10000000

int main()
{
   float * A;
   A=(float*)malloc(sizeof(float)*M*M);
   if (A==NULL)
   {
	printf("malloc A failed!\n");
	return 1;
   }

   float * B;
   B=(float*)malloc(sizeof(float)*M*M);

   float * C;
   C=(float*)malloc(sizeof(float)*M*M);

   float * C_ref;
   C_ref=(float*)malloc(sizeof(float)*M*M);


   FILE * f_input;
   f_input=fopen("A_input.txt","r");
   for (int i=0;i<M*M;i++)
   {
	fscanf (f_input, "%f", &A[i]);
   }
   fclose(f_input);

   f_input=fopen("B_input.txt","r");
   for (int i=0;i<M*M;i++)
   {
	fscanf (f_input, "%f", &B[i]);
   }
   fclose(f_input);

   f_input=fopen("C_ref.txt","r");
   for (int i=0;i<M*M;i++)
   {
	fscanf (f_input, "%f", &C_ref[i]);
   }
   fclose(f_input);


   cl_int error;
   cl_platform_id platform_id = NULL;
   cl_device_id device_id = NULL;

   cl_uint ret_num_devices;
   cl_uint ret_num_platforms;

   cl_context context = NULL;
   cl_command_queue command_queue = NULL;
   cl_program program = NULL;
   cl_kernel mm_kernel = NULL;

   
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

  //create program 
   FILE *fp = NULL;
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
   
   //printf("Kernel file read!\n");
   cl_int errnum, status;
   program = clCreateProgramWithBinary(context, 1, &device_id, &binary_length, (const unsigned char **)&binary, &status, &errnum);
   if (status != CL_SUCCESS || errnum != CL_SUCCESS)
   {
     printf("ERROR: Failed to create a program\n");
     return errnum;
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

   //create kernel
   mm_kernel = clCreateKernel(program, "matrixMult", &error);
   if (error != CL_SUCCESS) 
   {
	printf("Create kernel failed:%d\n",error);
        return 1;
   }


   //allocate argument arrays for the kernel
   cl_mem A_dev = NULL;
   cl_mem B_dev = NULL;
   cl_mem C_dev = NULL;


   A_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,sizeof(float)*M*M, NULL, &error);
   if (error != CL_SUCCESS) 
   {
	printf("A_dev create failed:%d\n",error);
        return 1;
   }

   B_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,sizeof(float)*M*M, NULL, &error);
   if (error != CL_SUCCESS) 
   {
	printf("B_dev create failed:%d\n",error);
        return 1;
   }

   C_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,sizeof(float)*M*M, NULL, &error);
   if (error != CL_SUCCESS) 
   {
	printf("C_dev create failed:%d\n",error);
        return 1;
   }


   //set arg for the kernel
   error = clSetKernelArg (mm_kernel,
  	0,
  	sizeof(cl_mem),
  	(void *) &C_dev);
   if (error != CL_SUCCESS) 
   {
	printf("Set arg C failed:%d\n",error);
        return 1;
   }


   error = clSetKernelArg (mm_kernel,
  	1,
  	sizeof(cl_mem),
  	(void *) &A_dev);
   if (error != CL_SUCCESS) 
   {
	printf("Set arg A failed:%d\n",error);
        return 1;
   }

   error = clSetKernelArg (mm_kernel,
  	2,
  	sizeof(cl_mem),
  	(void *) &B_dev);
   if (error != CL_SUCCESS) 
   {
	printf("Set arg B failed:%d\n",error);
        return 1;
   }
 
   int A_width=M;
   int B_width=M;


   error = clSetKernelArg (mm_kernel,
  	3,
  	sizeof(int),
  	(void *) &A_width);
   if (error != CL_SUCCESS) 
   {
	printf("Set arg A_width failed:%d\n",error);
        return 1;
   }


 
   


   //input arrays host -> device
   error=clEnqueueWriteBuffer (command_queue,
  	A_dev,
  	CL_TRUE,
  	0,
  	sizeof(float)*M*M,
  	A,
  	0,
  	NULL,
  	NULL);
   if (error != CL_SUCCESS)
   {
	printf("Write A_dev buffer failed:%d\n",error);
	return 1;
   }

   error=clEnqueueWriteBuffer (command_queue,
  	B_dev,
  	CL_TRUE,
  	0,
  	sizeof(float)*M*M,
  	B,
  	0,
  	NULL,
  	NULL);
   if (error != CL_SUCCESS)
   {
	printf("Write B_dev buffer failed:%d\n",error);
	return 1;
   }

   GET_TIME_INIT(2);
   GET_TIME_VAL(0);
   //execute the kernel

   size_t global_work_size[2];

   

   int GRIDDIM_X=M/BLOCKDIM/SUBDIM_X/SIMD_X;
   int GRIDDIM_Y=M/BLOCKDIM/SUBDIM_Y/SIMD_Y;

   global_work_size[0]=GRIDDIM_X*BLOCKDIM;
   global_work_size[1]=GRIDDIM_Y*BLOCKDIM;

   size_t local_work_size[2];
   local_work_size[0]=BLOCKDIM;
   local_work_size[1]=BLOCKDIM;

   //printf("before kernel starts!\n");
  
   for (int run_iter=0;run_iter<5;run_iter++)
   {

   	error = clEnqueueNDRangeKernel (command_queue,
  		mm_kernel,
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
   }

   GET_TIME_VAL(1);
   //results device -> host
   error = clEnqueueReadBuffer(command_queue, C_dev, CL_TRUE, 0,
			                sizeof(float)*M*M,C, 0, NULL, NULL);
   if (error != CL_SUCCESS) 
   {
	printf("C Device -> host failed:%d\n",error);
        return 1;
   }


   
   printf("Run-time is:%f ms \n",ELAPSED_TIME_MS(1, 0)/5);

   print_rsl;


   int precision_flag=0;

   FILE * f_output;
   f_output=fopen("C_out.txt","w");
   for (int i=0;i<M;i++)
   {
	for (int j=0;j<M;j++)
	{
          fprintf(f_output,"%f ",C[i*M+j]);
	  precision_flag+=(abs(C[i*M+j]-C_ref[i*M+j])>1)?1:0;
	}
	fprintf(f_output,"\n");
   }
   fclose(f_output);


   if (precision_flag)
	printf("Failed:Output does not match the golden out!!!!%d\n",precision_flag);
   else
        printf("Passed:Output matches the golden out!!!!\n");

}
