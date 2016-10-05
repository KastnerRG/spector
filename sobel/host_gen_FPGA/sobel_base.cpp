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
#include "CL/opencl.h"


#include "timer.h"






int main()
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
   cl_platform_id platform_id = NULL;
   cl_device_id device_id = NULL;

   cl_uint ret_num_devices;
   cl_uint ret_num_platforms;

   cl_context context = NULL;
   cl_command_queue command_queue = NULL;
   cl_program program = NULL;
   cl_kernel sobel_kernel = NULL;

   
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
   char file_name[] = KERNEL_NAME;
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
     printf("Failed to read window_filtering_aocl.aocx.\n");
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
   sobel_kernel = clCreateKernel(program, "sobel_filter", &error);
   if (error != CL_SUCCESS) 
   {
	printf("Create kernel failed:%d\n",error);
        return 1;
   }


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
