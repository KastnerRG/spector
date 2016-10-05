#include "CL/opencl.h"

class cl_host_module
{
	public:
		cl_int error;
   		cl_platform_id platform_id;
   		cl_device_id device_id;
   		cl_context context;
   		cl_command_queue command_queue;
   		cl_program program;
   		cl_kernel * kernel_array;
		
		cl_host_module();
	
		
};


cl_host_module::cl_host_module
{
	error=0;
   	platform_id = NULL;
   	device_id = NULL;
   	context = NULL;
   	command_queue = NULL;
   	program = NULL;
   	kernel_array = NULL;
}
