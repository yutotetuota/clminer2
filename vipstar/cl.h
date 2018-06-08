#ifndef _VIPSTARCOIN_CL_H_
#define _VIPSTARCOIN_CL_H_

#ifdef __cplusplus
extern "C"{
#endif

#include <stdbool.h>
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
	cl_uint max_compute_unit;
	char device_name[2048];
};

enum VENDOR{
	NVIDIA = 0x10de,
	AMD = 0x1002,
	INTEL = 0x8086
};

enum DISABLE_FLAG{
	DISABLE_NVIDIA = 1 << 0,
	DISABLE_AMD = 1 << 1,
	DISABLE_INTEL = 1 << 2,
	DISABLE_CPU = 1 << 3,
	DISABLE_GPU = 1 << 4,
	DISABLE_ACCELERATOR = 1 << 5,
	DISABLE_OTHER = 1 << 6
};

struct cl_ctx* create_cl_ctx();
int cl_init(struct cl_ctx *c, struct cl_device * const device, bool debug);
int cl_finish(struct cl_ctx *c);
void cl_release(struct cl_ctx *c);
int get_cl_device_count();
int get_cl_device(struct cl_device *devices, size_t buf_size);
const char* device_type_str(cl_device_type device_type);
bool is_disable(struct cl_device* const device, int disable_flag);
void show_device_info();
int cl_write_buffer(struct cl_ctx *c, const void *buf, size_t size);
int cl_release_buffer(struct cl_ctx *c);

#define CL_HASH_SIZE 8
#define CL_DATA_SIZE 128
#define CL_MIDSTATE_SIZE 8
#define CL_PREAHSH_SIZE 8
#define CL_BUF_SIZE 64

#ifdef __cplusplus
}
#endif

#endif /* _VIPSTARCOIN_CL_H_ */
