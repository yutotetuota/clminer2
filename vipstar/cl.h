#ifndef _VIPSTARCOIN_CL_H_
#define _VIPSTARCOIN_CL_H_

#include <stdint.h>
#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

struct cl_ctx {
	cl_uint num_cores;
	cl_uint work_group_size;
	cl_context context;
	cl_command_queue command_queue;
	cl_mem memobj;
	cl_program program;
	cl_kernel kernel;
	uint32_t buf_ptr;
	size_t buf_bytes;
	struct cl_device *device;
};

struct cl_device {
	cl_platform_id platform_id;
	cl_device_id device_id;
	cl_device_type device_type;
	cl_int vector_width;
	cl_uint vendor_id;
	char device_name[2048];
};

enum VENDOR{
	NVIDIA = 0x10de,
	AMD = 0x1002,
	INTEL = 0x8086
};

struct cl_ctx* create_cl_ctx();
int cl_init(struct cl_ctx *c, struct cl_device * const device);
int cl_finish(struct cl_ctx *c);
void cl_release(struct cl_ctx *c);
int get_cl_device_count();
int get_cl_device(struct cl_device *devices, size_t buf_size);
const char* device_type_str(struct cl_ctx *device);
void show_device_info();
int cl_write_buffer(struct cl_ctx *c, const void *buf, size_t size);
int cl_release_buffer(struct cl_ctx *c);

#define CL_HASH_SIZE 8
#define CL_DATA_SIZE 128
#define CL_MIDSTATE_SIZE 8
#define CL_PREAHSH_SIZE 8
#define CL_BUF_SIZE 64
#endif /* _VIPSTARCOIN_CL_H_ */
