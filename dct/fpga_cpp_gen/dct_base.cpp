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
 * Filename: dct_base.cpp
 * Version: 1.0
 * Description: DCT8x8 OpenCL benchmark.
 * Author: Pingfan Meng
 */

#include <math.h>
#include <stdio.h>
#include "stdlib.h"

#include "CL/opencl.h"

#include "oclDCT8x8_common.h"


#include "timer.h"

#define MAX_SOURCE_SIZE 100000000

#define NUM_ITER 1


#if BLOCK_SIZE_F == 1
#define BLOCK_SIZE 8

#elif BLOCK_SIZE_F== 2
#define BLOCK_SIZE 16

#elif BLOCK_SIZE_F== 4
#define BLOCK_SIZE 32

#endif

#define C_a 1.3870398453221474618216191915664f       //a = sqrt(2) * cos(1 * pi / 16)
#define C_b 1.3065629648763765278566431734272f       //b = sqrt(2) * cos(2 * pi / 16)
#define C_c 1.1758756024193587169744671046113f       //c = sqrt(2) * cos(3 * pi / 16)
#define C_d 0.78569495838710218127789736765722f      //d = sqrt(2) * cos(5 * pi / 16)
#define C_e 0.54119610014619698439972320536639f      //e = sqrt(2) * cos(6 * pi / 16)
#define C_f 0.27589937928294301233595756366937f      //f = sqrt(2) * cos(7 * pi / 16)
#define C_norm 0.35355339059327376220042218105242f   //1 / sqrt(8)




int DCT8x8_CL_LAUNCHER(float *dst, float *src, cl_uint stride, cl_uint imageH, cl_uint imageW)
{
	cl_int error;
	cl_platform_id platform_id = NULL;
	cl_device_id device_id = NULL;

	cl_uint ret_num_devices;
	cl_uint ret_num_platforms;

	cl_context context = NULL;
	cl_command_queue command_queue = NULL;
	cl_program program = NULL;
	cl_kernel dct_kernel = NULL;

	FILE *fp;
	char fileName[] = CL_FILE_NAME;
	char *source_str;
	size_t source_size;


	fp = fopen(fileName, "r");
	if (!fp) {
		printf("Failed to load kernel file.\n");
		return 1;
	}
	source_str = (char*)malloc(MAX_SOURCE_SIZE);
	if (source_str==NULL)
	{
	   	printf("malloc source_str failed!\n");
	   	return 1;
	}

	source_size = fread(source_str, 1, MAX_SOURCE_SIZE, fp);
	fclose(fp);

	if (source_str==NULL)
	{
		printf("read file failed!\n");
		return 1;
	}
	   
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
   program = clCreateProgramWithSource(context, 1, (const char **)&source_str,
				                      (const size_t *)&source_size, &error);

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

   //create kernel
   dct_kernel = clCreateKernel(program, "DCT8x8", &error);
   if (error != CL_SUCCESS) 
   {
	printf("Create kernel failed:%d\n",error);
        return 1;
   }

   //allocate argument arrays for the kernel
   cl_mem dst_dev = NULL;
   cl_mem src_dev= NULL;


   dst_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,imageH * stride *sizeof(float), NULL, &error);
   if (error != CL_SUCCESS) 
   {
	printf("dst_dev create failed:%d\n",error);
        return 1;
   }

   src_dev = clCreateBuffer(context, CL_MEM_READ_WRITE,imageH * stride *sizeof(float), NULL, &error);
   if (error != CL_SUCCESS) 
   {
	printf("src_dev create failed:%d\n",error);
        return 1;
   }

 
   //set arg for the kernel
   error = clSetKernelArg (dct_kernel,
  	0,
  	sizeof(cl_mem),
  	(void *) &dst_dev);
   if (error != CL_SUCCESS) 
   {
	printf("Set arg dst failed:%d\n",error);
        return 1;
   }


   error = clSetKernelArg (dct_kernel,
  	1,
  	sizeof(cl_mem),
  	(void *) &src_dev);
   if (error != CL_SUCCESS) 
   {
	printf("Set arg src failed:%d\n",error);
        return 1;
   }


   error = clSetKernelArg (dct_kernel,
  	2,
  	sizeof(cl_uint),
  	(void *) &stride);
   if (error != CL_SUCCESS) 
   {
	printf("Set arg stride failed:%d\n",error);
        return 1;
   }

   error = clSetKernelArg (dct_kernel,
  	3,
  	sizeof(cl_uint),
  	(void *) &imageH);
   if (error != CL_SUCCESS) 
   {
	printf("Set arg imageH failed:%d\n",error);
        return 1;
   }

   error = clSetKernelArg (dct_kernel,
  	4,
  	sizeof(cl_uint),
  	(void *) &imageW);
   if (error != CL_SUCCESS) 
   {
	printf("Set arg imageW failed:%d\n",error);
        return 1;
   }

   
   //input arrays host -> device
   error=clEnqueueWriteBuffer (command_queue,
  	src_dev,
  	CL_TRUE,
  	0,
  	imageH * stride *sizeof(float),
  	src,
  	0,
  	NULL,
  	NULL);
   if (error != CL_SUCCESS)
   {
	printf("Write src_dev buffer failed:%d\n",error);
	return 1;
   }

   GET_TIME_INIT(2);
   GET_TIME_VAL(0);
   //execute the kernel

   size_t global_work_size[2];

   

   //int GRIDDIM_X=imageW/BLOCKDIM_X/SUBDIM_X/SIMD_X;
   //int GRIDDIM_Y=imageH/BLOCKDIM_Y/SUBDIM_Y/SIMD_Y;

   

   size_t local_work_size[2];

   #if SIMD_TYPE == 0
   local_work_size[0]=BLOCKDIM_X/SIMD_LOC;
   local_work_size[1]=BLOCKDIM_Y/BLOCK_SIZE;

   global_work_size[0]=imageW/BLOCKDIM_X*local_work_size[0];
   global_work_size[1]=imageH/BLOCKDIM_Y*local_work_size[1];



   #elif SIMD_TYPE == 1
   local_work_size[0]=BLOCKDIM_X;
   local_work_size[1]=BLOCKDIM_Y/BLOCK_SIZE/SIMD_LOC;

   global_work_size[0]=imageW/BLOCKDIM_X*local_work_size[0];
   global_work_size[1]=imageH/BLOCKDIM_Y*local_work_size[1];

   #endif


   for (int iter=0; iter<NUM_ITER; iter++)
   {

   	error = clEnqueueNDRangeKernel (command_queue,
  		dct_kernel,
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
   error = clEnqueueReadBuffer(command_queue, dst_dev, CL_TRUE, 0,
			                imageH * stride *sizeof(float),dst, 0, NULL, NULL);
   if (error != CL_SUCCESS) 
   {
	printf("dst Device -> host failed:%d\n",error);
        return 1;
   }


   
   printf("Run-time is:%f ms \n",ELAPSED_TIME_MS(1, 0)/NUM_ITER);


   print_rsl;
	
  //release everything on the device
   clReleaseMemObject(dst_dev);	
   clReleaseMemObject(src_dev);	

   clReleaseKernel(dct_kernel);

   return 0;

}

int main()
{
	float *h_Input, *h_OutputCPU, *h_OutputAcc;

	const cl_uint
        imageW = 2048,
        imageH = 2048,
        stride = 2048;

	const int dir = DCT_FORWARD;
	

	h_Input     = (float *)malloc(imageH * stride * sizeof(float));
        h_OutputCPU = (float *)malloc(imageH * stride * sizeof(float));
        h_OutputAcc = (float *)malloc(imageH * stride * sizeof(float));
        srand(2009);
        for(cl_uint i = 0; i < imageH; i++)
            for(cl_uint j = 0; j < imageW; j++)
                h_Input[i * stride + j] = (float)rand() / (float)RAND_MAX;



	//compute the reference results
	DCT8x8CPU(h_OutputCPU, h_Input, stride, imageH, imageW, dir);

	//compute on the device
	DCT8x8_CL_LAUNCHER(h_OutputAcc, h_Input, stride, imageH, imageW);


	//compare the error
	FILE * ref_file;
	FILE * output_file;
	ref_file=fopen("cpu_ref_output.txt","w");
	output_file=fopen("device_output.txt","w");
	double sum = 0, delta = 0;
        double L2norm;
	for(cl_uint i = 0; i < imageH; i++)
	{
            for(cl_uint j = 0; j < imageW; j++){
                sum += h_OutputCPU[i * stride + j] * h_OutputCPU[i * stride + j];
                delta += (h_OutputAcc[i * stride + j] - h_OutputCPU[i * stride + j]) * (h_OutputAcc[i * stride + j] - h_OutputCPU[i * stride + j]);
		fprintf(ref_file,"%f ",h_OutputCPU[i * stride + j]);
		fprintf(output_file,"%f ",h_OutputAcc[i * stride + j]);
            }
		fprintf(ref_file,"\n ");
		fprintf(output_file,"\n ");
	}
        L2norm = sqrt(delta / sum);
        printf("Relative L2 norm: %.3e\n\n", L2norm);
       
        if (L2norm <0.001)
	{
		printf("PASSED!\n");
	}
	else 
	{
		printf("FAILED!\n");
	}

	fclose(ref_file);
	fclose(output_file);

	



	free(h_Input);
	free(h_OutputCPU);
	free(h_OutputAcc);


	return 0;
}
