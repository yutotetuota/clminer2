#include <stdio.h>
#include <stdlib.h>
#include "cl.h"
#include "../miner.h"

#define OCLSTRINGIFY(...) #__VA_ARGS__


const char *source_str =
		#include "./sha256d.cl"
	;

size_t source_size = 
	sizeof(
		#include "./sha256d.cl"
	);

void show_device_info(){
	cl_int ret;
	cl_uint num_platforms;

	ret = clGetPlatformIDs(114514, NULL, &num_platforms);
	cl_platform_id *platform_id = (cl_platform_id*)calloc(num_platforms, sizeof(cl_platform_id));
	ret = clGetPlatformIDs(num_platforms, platform_id, NULL);

	if(num_platforms == 0){
		printf("OpenCL device not found.\n");
		return;
	}

	size_t offset = 0;
	for(size_t i = 0; i < num_platforms; i++){
		cl_uint num_devices = 0;
		ret = clGetDeviceIDs(platform_id[i], CL_DEVICE_TYPE_ALL, 0, NULL, &num_devices);

		cl_device_id *device_id = (cl_device_id*)calloc(num_devices, sizeof(cl_device_id));
		ret = clGetDeviceIDs(platform_id[i], CL_DEVICE_TYPE_ALL, num_devices, device_id, &num_devices);

		for(size_t j = 0; j < num_devices; j++){
			char device_name[1024] = { 0 };
			char vendor_name[1024] = { 0 };
			char platform_name[1024] = { 0 };
			char opencl_version[1024] = { 0 };
			ret = clGetDeviceInfo(device_id[j], CL_DEVICE_NAME, sizeof(device_name), device_name, NULL);
			ret = clGetDeviceInfo(device_id[j], CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, NULL);
			ret = clGetDeviceInfo(device_id[j], CL_DEVICE_VERSION, sizeof(opencl_version), opencl_version, NULL);
			ret = clGetPlatformInfo(platform_id[i], CL_PLATFORM_NAME, sizeof(platform_name), platform_name, NULL);
			
			printf("Device ID:%d\n  Platform name\t\t%s\n  Device name\t\t%s\n  Device vendor\t\t%s\n  OpenCL version\t%s\n\n", (int)(offset + j), platform_name, device_name, vendor_name, opencl_version);
		}
		offset += num_devices;
		free(device_id);
	}
}

int get_cl_device_count(){
	cl_int ret;
	cl_uint num_platforms;
	ret = clGetPlatformIDs(114514, NULL, &num_platforms); if(ret != CL_SUCCESS) return ret;
	cl_platform_id *platform_id = (cl_platform_id*)calloc(num_platforms, sizeof(cl_platform_id));

	ret = clGetPlatformIDs(num_platforms, platform_id, NULL); if(ret != CL_SUCCESS) return ret;

	cl_uint devices = 0;
	cl_uint total_devices = 0;
	for(size_t i = 0; i < num_platforms; i++){
		ret = clGetDeviceIDs(platform_id[i], CL_DEVICE_TYPE_ALL, 0, NULL, &devices); if(ret != CL_SUCCESS) return ret;
		total_devices += devices;
	}

	free(platform_id);

	return total_devices;
}

int get_cl_device(struct cl_device *devices, size_t num_entries){
	cl_int ret;
	cl_uint num_platforms;

	ret = clGetPlatformIDs(114514, NULL, &num_platforms); if(ret != CL_SUCCESS) return ret;
	cl_platform_id *platform_id = (cl_platform_id*)calloc(num_platforms, sizeof(cl_platform_id));
	ret = clGetPlatformIDs(num_platforms, platform_id, NULL); if(ret != CL_SUCCESS) return ret;


	int offset = 0;
	for(size_t i = 0; i < num_platforms; i++){
		cl_uint num_devices = 0;		
		ret = clGetDeviceIDs(platform_id[i], CL_DEVICE_TYPE_ALL, (cl_uint)num_entries, NULL, &num_devices); if(ret != CL_SUCCESS) return ret;

		cl_device_id *device_id = (cl_device_id*)calloc(num_devices, sizeof(cl_device_id));
		ret = clGetDeviceIDs(platform_id[i], CL_DEVICE_TYPE_ALL, (cl_uint)num_entries, device_id, &num_devices); if(ret != CL_SUCCESS) return ret;

		for(size_t j = 0; j < num_devices; j++){
			devices[offset + j].platform_id = platform_id[i];
			devices[offset + j].device_id =  device_id[j];

			char device_name[1024] = { 0 };
			char vendor_name[1023] = { 0 };
			ret = clGetDeviceInfo(device_id[j], CL_DEVICE_NAME, sizeof(device_name), device_name, NULL); if(ret != CL_SUCCESS) return ret;
			ret = clGetDeviceInfo(device_id[j], CL_DEVICE_VENDOR, sizeof(vendor_name), vendor_name, NULL); if(ret != CL_SUCCESS) return ret;

			sprintf(devices[offset + j].device_name , "%s %s", vendor_name, device_name);

			ret = clGetDeviceInfo(device_id[j], CL_DEVICE_NATIVE_VECTOR_WIDTH_INT, sizeof(cl_int), &devices[offset + j].vector_width, NULL); if(ret != CL_SUCCESS) return ret;
			ret = clGetDeviceInfo(device_id[j], CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &devices[offset + j].vendor_id, NULL); if(ret != CL_SUCCESS) return ret;
			ret = clGetDeviceInfo(device_id[j], CL_DEVICE_TYPE, sizeof(cl_device_type), &devices[offset + j].device_type, NULL); if(ret != CL_SUCCESS) return ret;
		}

		free(device_id);
		offset += num_devices;
	}

	free(platform_id);

	return ret;
}

const char* device_type_str(struct cl_ctx *ctx){
	switch(ctx->device->device_type){
		case CL_DEVICE_TYPE_CPU: return "CPU";
		case CL_DEVICE_TYPE_GPU: return "GPU";
		case CL_DEVICE_TYPE_ACCELERATOR: return "ACCELERATOR";
		default: return "CPU";
	}
}

struct cl_ctx* create_cl_ctx(){
	return (struct cl_ctx*)calloc(1, sizeof(struct cl_ctx));
}


int cl_init(struct cl_ctx *c, struct cl_device* const device){
	cl_int ret;

	c->device = device;
	
	char option[256] = { 0 };

	if(device->vendor_id == NVIDIA && device->device_type == CL_DEVICE_TYPE_GPU){
		sprintf(option, "-cl-nv-maxrregcount=128 -D VECTOR_WIDTH_%d", device->vector_width);
	} else {
		sprintf(option, "-D VECTOR_WIDTH_%d", device->vector_width);
	} 

	c->context = clCreateContext( NULL, 1, &device->device_id, NULL, NULL, &ret); if(ret != CL_SUCCESS) return ret;
	c->command_queue = clCreateCommandQueue(c->context, device->device_id, CL_QUEUE_PROFILING_ENABLE, &ret); if(ret != CL_SUCCESS) return ret;
	c->program = clCreateProgramWithSource(c->context, 1, (const char **)&source_str, &source_size, &ret); if(ret != CL_SUCCESS) return ret;
	ret = clBuildProgram(c->program, 1, &device->device_id, option, NULL, NULL); if(ret != CL_SUCCESS) return ret;


	if(ret) {
		size_t log_size;
		clGetProgramBuildInfo(c->program, device->device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
		char *log = (char *) malloc(log_size);
		clGetProgramBuildInfo(c->program, device->device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
		printf("error %s\n", log);
		free(log);
		if(ret != CL_SUCCESS) return ret;
	}

	c->memobj = clCreateBuffer(c->context, CL_MEM_READ_WRITE, sizeof(uint32_t) * (4 + CL_HASH_SIZE + CL_DATA_SIZE + CL_MIDSTATE_SIZE), NULL, &ret); if(ret != CL_SUCCESS) return ret;
	c->kernel = clCreateKernel(c->program, "sha256d_vips_ms_cl", &ret); if(ret != CL_SUCCESS) return ret;
	ret = clSetKernelArg(c->kernel, 0, sizeof(cl_mem), (void*)&c->memobj); if(ret != CL_SUCCESS) return ret;


	size_t work_group_size_multiple;
	ret = clGetKernelWorkGroupInfo(c->kernel, device->device_id, CL_KERNEL_PREFERRED_WORK_GROUP_SIZE_MULTIPLE, sizeof(size_t), &work_group_size_multiple, NULL); if(ret != CL_SUCCESS) return ret;


	cl_int compute_units;
	ret = clGetDeviceInfo(device->device_id, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_int), &compute_units, NULL); if(ret != CL_SUCCESS) return ret;

	switch(device->vendor_id){
		case NVIDIA :
			{
				switch(device->device_type){
					case CL_DEVICE_TYPE_GPU: 
						c->num_cores = compute_units * 32 * 4 * 64; 
						c->work_group_size = 32; 
						break;
					case CL_DEVICE_TYPE_CPU: 
						c->num_cores = compute_units * 32;
						c->work_group_size = 1; 
						break;
					case CL_DEVICE_TYPE_ACCELERATOR:
						c->num_cores = compute_units * (cl_int)work_group_size_multiple;
						c->work_group_size = (cl_int)work_group_size_multiple;
						break;
					default:
						c->num_cores = compute_units * (cl_int)work_group_size_multiple;
						c->work_group_size = (cl_int)work_group_size_multiple;
						break;
				}
			}
			break;
		case AMD :
			{
				switch(device->device_type){
					case CL_DEVICE_TYPE_GPU: 
						c->num_cores = compute_units * 64 * 32; 
						c->work_group_size = 64; 
						break;
					case CL_DEVICE_TYPE_CPU: 
						c->num_cores = compute_units * 32;
						c->work_group_size = 1; 
						break;
					case CL_DEVICE_TYPE_ACCELERATOR:
						c->num_cores = compute_units * (cl_int)work_group_size_multiple;
						c->work_group_size = (cl_int)work_group_size_multiple;
						break;
					default:
						c->num_cores = compute_units * (cl_int)work_group_size_multiple;
						c->work_group_size = (cl_int)work_group_size_multiple;
						break;
				}
			}
			break;
		case INTEL :
			{
				switch(device->device_type){
					//https://software.intel.com/sites/default/files/managed/c5/9a/The-Compute-Architecture-of-Intel-Processor-Graphics-Gen9-v1d0.pdf
					case CL_DEVICE_TYPE_GPU: 
						c->num_cores = compute_units * 7 * 32; 
						c->work_group_size = 7;
						break;
					case CL_DEVICE_TYPE_CPU: 
						c->num_cores = compute_units * 32;
						c->work_group_size = 1; 
						break;
					case CL_DEVICE_TYPE_ACCELERATOR:
						c->num_cores = compute_units * (cl_int)work_group_size_multiple;
						c->work_group_size = (cl_int)work_group_size_multiple;
						break;
					default:
						c->num_cores = compute_units * (cl_int)work_group_size_multiple;
						c->work_group_size = (cl_int)work_group_size_multiple;
						break;
				}
			}
			break;
		default :
			{
				switch(device->device_type){
					case CL_DEVICE_TYPE_GPU: 
						c->num_cores = compute_units * (cl_int)work_group_size_multiple * 32;
						c->work_group_size = 7; 
						break;
					case CL_DEVICE_TYPE_CPU: 
						c->num_cores = compute_units * 32;
						c->work_group_size = 1; 
						break;
					case CL_DEVICE_TYPE_ACCELERATOR:
						c->num_cores = compute_units * (cl_int)work_group_size_multiple;
						c->work_group_size = (cl_int)work_group_size_multiple;
						break;
					default:
						c->num_cores = compute_units * (cl_int)work_group_size_multiple;
						c->work_group_size = (cl_int)work_group_size_multiple;
						break;
				}
			}
			break;
	}

	return ret;
}

int cl_finish(struct cl_ctx *c){
	cl_int ret;

	ret = clFlush(c->command_queue);
	ret = clFinish(c->command_queue);
	ret = clReleaseKernel(c->kernel);
	ret = clReleaseProgram(c->program);
	ret = clReleaseMemObject(c->memobj);
	ret = clReleaseCommandQueue(c->command_queue);
	ret = clReleaseContext(c->context);

	return ret;
}

void cl_release(struct cl_ctx *c){
	free(c);
}

int cl_write_buffer(struct cl_ctx *c, const void *buf, size_t size){
	cl_int ret;
	ret = clEnqueueWriteBuffer(c->command_queue, c->memobj, CL_TRUE, 0, size, buf, 0, NULL, NULL);
	
	return ret;
}

int cl_release_buffer(struct cl_ctx *c){
	cl_int ret;
	ret = clReleaseMemObject(c->memobj);

	return ret;
} 