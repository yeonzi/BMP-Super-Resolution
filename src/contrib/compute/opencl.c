/******************************************************************************
The MIT License
Copyright (c) 2017-2018 Yeonji
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
******************************************************************************/

#include <contrib/compute/compute_private.h>
#include <contrib/compute/compute.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_PLATFORMS 4
#define MAX_DEVICES   8

int opencl_available_status = 0;

cl_platform_id      platform_selected;
cl_device_id        device_selected;
cl_context          context;
cl_program          program;
cl_command_queue    queue;

extern char opencl_program_str[];
extern size_t opencl_program_str_len;

static char * string_cl_success                                  = \
"OpenCL success";
static char * string_cl_device_not_found                         = \
"OpenCL device not found";
static char * string_cl_device_not_available                     = \
"OpenCL device not available";
static char * string_cl_compiler_not_available                   = \
"OpenCL compiler not available";
static char * string_cl_mem_object_allocation_failure            = \
"OpenCL mem object allocation failure";
static char * string_cl_out_of_resources                         = \
"OpenCL out of resources";
static char * string_cl_out_of_host_memory                       = \
"OpenCL out of host memory";
static char * string_cl_profiling_info_not_available             = \
"OpenCL profiling info not available";
static char * string_cl_mem_copy_overlap                         = \
"OpenCL mem copy overlap";
static char * string_cl_image_format_mismatch                    = \
"OpenCL image format mismatch";
static char * string_cl_image_format_not_supported               = \
"OpenCL image format not supported";
static char * string_cl_build_program_failure                    = \
"OpenCL build program failure";
static char * string_cl_map_failure                              = \
"OpenCL map failure";
static char * string_cl_misaligned_sub_buffer_offset             = \
"OpenCL misaligned sub buffer offset";
static char * string_cl_exec_status_error_for_events_in_wait_list= \
"OpenCL exec status error for events in wait list";
static char * string_cl_compile_program_failure                  = \
"OpenCL compile program failure";
static char * string_cl_linker_not_available                     = \
"OpenCL linker not available";
static char * string_cl_link_program_failure                     = \
"OpenCL link program failure";
static char * string_cl_device_partition_failed                  = \
"OpenCL device partition failed";
static char * string_cl_kernel_arg_info_not_available            = \
"OpenCL kernel arg info not available";
static char * string_cl_invalid_value                            = \
"OpenCL invalid value";
static char * string_cl_invalid_device_type                      = \
"OpenCL invalid device type";
static char * string_cl_invalid_platform                         = \
"OpenCL invalid platform";
static char * string_cl_invalid_device                           = \
"OpenCL invalid device";
static char * string_cl_invalid_context                          = \
"OpenCL invalid context";
static char * string_cl_invalid_queue_properties                 = \
"OpenCL invalid queue properties";
static char * string_cl_invalid_command_queue                    = \
"OpenCL invalid command queue";
static char * string_cl_invalid_host_ptr                         = \
"OpenCL invalid host ptr";
static char * string_cl_invalid_mem_object                       = \
"OpenCL invalid mem object";
static char * string_cl_invalid_image_format_descriptor          = \
"OpenCL invalid image format descriptor";
static char * string_cl_invalid_image_size                       = \
"OpenCL invalid image size";
static char * string_cl_invalid_sampler                          = \
"OpenCL invalid sampler";
static char * string_cl_invalid_binary                           = \
"OpenCL invalid binary";
static char * string_cl_invalid_build_options                    = \
"OpenCL invalid build options";
static char * string_cl_invalid_program                          = \
"OpenCL invalid program";
static char * string_cl_invalid_program_executable               = \
"OpenCL invalid program executable";
static char * string_cl_invalid_kernel_name                      = \
"OpenCL invalid kernel name";
static char * string_cl_invalid_kernel_definition                = \
"OpenCL invalid kernel definition";
static char * string_cl_invalid_kernel                           = \
"OpenCL invalid kernel";
static char * string_cl_invalid_arg_index                        = \
"OpenCL invalid arg index";
static char * string_cl_invalid_arg_value                        = \
"OpenCL invalid arg value";
static char * string_cl_invalid_arg_size                         = \
"OpenCL invalid arg size";
static char * string_cl_invalid_kernel_args                      = \
"OpenCL invalid kernel args";
static char * string_cl_invalid_work_dimension                   = \
"OpenCL invalid work dimension";
static char * string_cl_invalid_work_group_size                  = \
"OpenCL invalid work group size";
static char * string_cl_invalid_work_item_size                   = \
"OpenCL invalid work item size";
static char * string_cl_invalid_global_offset                    = \
"OpenCL invalid global offset";
static char * string_cl_invalid_event_wait_list                  = \
"OpenCL invalid event wait list";
static char * string_cl_invalid_event                            = \
"OpenCL invalid event";
static char * string_cl_invalid_operation                        = \
"OpenCL invalid operation";
static char * string_cl_invalid_gl_object                        = \
"OpenCL invalid gl object";
static char * string_cl_invalid_buffer_size                      = \
"OpenCL invalid buffer size";
static char * string_cl_invalid_mip_level                        = \
"OpenCL invalid mip level";
static char * string_cl_invalid_global_work_size                 = \
"OpenCL invalid global work size";
static char * string_cl_invalid_property                         = \
"OpenCL invalid property";
static char * string_cl_invalid_image_descriptor                 = \
"OpenCL invalid image descriptor";
static char * string_cl_invalid_compiler_options                 = \
"OpenCL invalid compiler options";
static char * string_cl_invalid_linker_options                   = \
"OpenCL invalid linker options";
static char * string_cl_invalid_device_partition_count           = \
"OpenCL invalid device partition count";

const char * opencl_get_error_string(int err)
{
    switch (err) {
        case CL_SUCCESS:
            return string_cl_success;
            break;
        case CL_DEVICE_NOT_FOUND:
            return string_cl_device_not_found;
            break;
        case CL_DEVICE_NOT_AVAILABLE:
            return string_cl_device_not_available;
            break;
        case CL_COMPILER_NOT_AVAILABLE:
            return string_cl_compiler_not_available;
            break;
        case CL_MEM_OBJECT_ALLOCATION_FAILURE:
            return string_cl_mem_object_allocation_failure;
            break;
        case CL_OUT_OF_RESOURCES:
            return string_cl_out_of_resources;
            break;
        case CL_OUT_OF_HOST_MEMORY:
            return string_cl_out_of_host_memory;
            break;
        case CL_PROFILING_INFO_NOT_AVAILABLE:
            return string_cl_profiling_info_not_available;
            break;
        case CL_MEM_COPY_OVERLAP:
            return string_cl_mem_copy_overlap;
            break;
        case CL_IMAGE_FORMAT_MISMATCH:
            return string_cl_image_format_mismatch;
            break;
        case CL_IMAGE_FORMAT_NOT_SUPPORTED:
            return string_cl_image_format_not_supported;
            break;
        case CL_BUILD_PROGRAM_FAILURE:
            return string_cl_build_program_failure;
            break;
        case CL_MAP_FAILURE:
            return string_cl_map_failure;
            break;
        case CL_MISALIGNED_SUB_BUFFER_OFFSET:
            return string_cl_misaligned_sub_buffer_offset;
            break;
        case CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST:
            return string_cl_exec_status_error_for_events_in_wait_list;
            break;
        case CL_COMPILE_PROGRAM_FAILURE:
            return string_cl_compile_program_failure;
            break;
        case CL_LINKER_NOT_AVAILABLE:
            return string_cl_linker_not_available;
            break;
        case CL_LINK_PROGRAM_FAILURE:
            return string_cl_link_program_failure;
            break;
        case CL_DEVICE_PARTITION_FAILED:
            return string_cl_device_partition_failed;
            break;
        case CL_KERNEL_ARG_INFO_NOT_AVAILABLE:
            return string_cl_kernel_arg_info_not_available;
            break;
        case CL_INVALID_VALUE:
            return string_cl_invalid_value;
            break;
        case CL_INVALID_DEVICE_TYPE:
            return string_cl_invalid_device_type;
            break;
        case CL_INVALID_PLATFORM:
            return string_cl_invalid_platform;
            break;
        case CL_INVALID_DEVICE:
            return string_cl_invalid_device;
            break;
        case CL_INVALID_CONTEXT:
            return string_cl_invalid_context;
            break;
        case CL_INVALID_QUEUE_PROPERTIES:
            return string_cl_invalid_queue_properties;
            break;
        case CL_INVALID_COMMAND_QUEUE:
            return string_cl_invalid_command_queue;
            break;
        case CL_INVALID_HOST_PTR:
            return string_cl_invalid_host_ptr;
            break;
        case CL_INVALID_MEM_OBJECT:
            return string_cl_invalid_mem_object;
            break;
        case CL_INVALID_IMAGE_FORMAT_DESCRIPTOR:
            return string_cl_invalid_image_format_descriptor;
            break;
        case CL_INVALID_IMAGE_SIZE:
            return string_cl_invalid_image_size;
            break;
        case CL_INVALID_SAMPLER:
            return string_cl_invalid_sampler;
            break;
        case CL_INVALID_BINARY:
            return string_cl_invalid_binary;
            break;
        case CL_INVALID_BUILD_OPTIONS:
            return string_cl_invalid_build_options;
            break;
        case CL_INVALID_PROGRAM:
            return string_cl_invalid_program;
            break;
        case CL_INVALID_PROGRAM_EXECUTABLE:
            return string_cl_invalid_program_executable;
            break;
        case CL_INVALID_KERNEL_NAME:
            return string_cl_invalid_kernel_name;
            break;
        case CL_INVALID_KERNEL_DEFINITION:
            return string_cl_invalid_kernel_definition;
            break;
        case CL_INVALID_KERNEL:
            return string_cl_invalid_kernel;
            break;
        case CL_INVALID_ARG_INDEX:
            return string_cl_invalid_arg_index;
            break;
        case CL_INVALID_ARG_VALUE:
            return string_cl_invalid_arg_value;
            break;
        case CL_INVALID_ARG_SIZE:
            return string_cl_invalid_arg_size;
            break;
        case CL_INVALID_KERNEL_ARGS:
            return string_cl_invalid_kernel_args;
            break;
        case CL_INVALID_WORK_DIMENSION:
            return string_cl_invalid_work_dimension;
            break;
        case CL_INVALID_WORK_GROUP_SIZE:
            return string_cl_invalid_work_group_size;
            break;
        case CL_INVALID_WORK_ITEM_SIZE:
            return string_cl_invalid_work_item_size;
            break;
        case CL_INVALID_GLOBAL_OFFSET:
            return string_cl_invalid_global_offset;
            break;
        case CL_INVALID_EVENT_WAIT_LIST:
            return string_cl_invalid_event_wait_list;
            break;
        case CL_INVALID_EVENT:
            return string_cl_invalid_event;
            break;
        case CL_INVALID_OPERATION:
            return string_cl_invalid_operation;
            break;
        case CL_INVALID_GL_OBJECT:
            return string_cl_invalid_gl_object;
            break;
        case CL_INVALID_BUFFER_SIZE:
            return string_cl_invalid_buffer_size;
            break;
        case CL_INVALID_MIP_LEVEL:
            return string_cl_invalid_mip_level;
            break;
        case CL_INVALID_GLOBAL_WORK_SIZE:
            return string_cl_invalid_global_work_size;
            break;
        case CL_INVALID_PROPERTY:
            return string_cl_invalid_property;
            break;
        case CL_INVALID_IMAGE_DESCRIPTOR:
            return string_cl_invalid_image_descriptor;
            break;
        case CL_INVALID_COMPILER_OPTIONS:
            return string_cl_invalid_compiler_options;
            break;
        case CL_INVALID_LINKER_OPTIONS:
            return string_cl_invalid_linker_options;
            break;
        case CL_INVALID_DEVICE_PARTITION_COUNT:
            return string_cl_invalid_device_partition_count;
            break;
        default:
            return string_cl_success;
            break;
    }
}

int opencl_available(void)
{
    extern int opencl_available_status;
    return opencl_available_status;
}

int opencl_list(void)
{
    cl_int              err;

    cl_platform_id      platforms[MAX_PLATFORMS];
    cl_uint             platform_cnt;
    unsigned int        platform_index;

    cl_device_id        devices[MAX_PLATFORMS][MAX_DEVICES];
    cl_uint             device_cnt[MAX_PLATFORMS];
    unsigned int        device_index;

    char vendor[100];
    char name[100];

    int cnt = 0;

    /* Identify a platform */
    err = clGetPlatformIDs(MAX_PLATFORMS, platforms, &platform_cnt);

    if(platform_cnt <= 0) {
        fprintf(stderr, "No OpenCL Platform Available.\n");
      return 0;
    } else {
        fprintf(stderr, "%d OpenCL Platform Found.\n", platform_cnt);
    }

    for (platform_index = 0; platform_index < platform_cnt; platform_index++) {
        err |= clGetDeviceIDs( platforms[platform_index], \
                              CL_DEVICE_TYPE_ALL, 5, \
                              devices[platform_index], \
                              &device_cnt[platform_index] );
    }

    for (platform_index = 0; platform_index < platform_cnt; platform_index++) {
        err |= clGetPlatformInfo( platforms[platform_index], CL_PLATFORM_VENDOR,\
            sizeof(vendor), vendor ,NULL);
        err |= clGetPlatformInfo( platforms[platform_index], CL_PLATFORM_NAME, \
            sizeof(name), name ,NULL);
        fprintf(stderr, "Platform %d: %s %s (%d Devices)\n", platform_index, \
            vendor, name, device_cnt[platform_index]);
    }

    fprintf(stderr, "Device 0: Native code (without any accelerator)\n");

    for (platform_index = 0; platform_index < platform_cnt; platform_index++) {
        for (device_index = 0; \
            device_index < device_cnt[platform_index]; \
            device_index++) {
            cnt ++;
            err |= clGetDeviceInfo( devices[platform_index][device_index], \
                        CL_DEVICE_VENDOR, sizeof(vendor), vendor ,NULL);
            err |= clGetDeviceInfo( devices[platform_index][device_index], \
                        CL_DEVICE_NAME, sizeof(name), name ,NULL);
            fprintf(stderr, "Device %d: %s %s (on OpenCL Platfrom %d)\n", \
                cnt, vendor, name, platform_index);
        }
    }

    return err;
}

int opencl_init(unsigned int device_id)
{
    cl_int              err;

    cl_platform_id      platforms[MAX_PLATFORMS];
    cl_uint             platform_cnt;
    unsigned int        platform_index;

    cl_device_id        devices[MAX_PLATFORMS][MAX_DEVICES];
    cl_uint             device_cnt[MAX_PLATFORMS];

    char vendor[100];
    char name[100];

#if defined (CL_VERSION_2_0)

    cl_queue_properties prop_default[] = {  CL_QUEUE_PROPERTIES, \
        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | \
        CL_QUEUE_ON_DEVICE|CL_QUEUE_ON_DEVICE_DEFAULT, 0 };
    cl_queue_properties prop_inorder[] = {  CL_QUEUE_PROPERTIES, \
        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE | \
        CL_QUEUE_ON_DEVICE|CL_QUEUE_ON_DEVICE_DEFAULT, 0 };
    cl_queue_properties prop_host[] = {  CL_QUEUE_PROPERTIES, \
        CL_QUEUE_PROFILING_ENABLE, 0 };

#endif

    unsigned int cnt = 0;

    extern int              opencl_available_status;
    extern cl_platform_id   platform_selected;
    extern cl_device_id     device_selected;
    extern cl_context       context;
    extern cl_program       program;
    extern cl_command_queue queue;

    char * program_ptr;
    char cl_compile_options[] = "-cl-mad-enable -cl-fast-relaxed-math";
    char * compile_log;
    size_t log_size;

    if (device_id == 0) {
        fprintf(stderr, "Native code selected.\n");
        return 0;
    }

    /* Identify a platform */
    err = clGetPlatformIDs(MAX_PLATFORMS, platforms, &platform_cnt);

    if(platform_cnt <= 0) {
        fprintf(stderr, "Invalid device id.\n");
        fprintf(stderr, "Native code selected.\n");
        return 0;
    }

    for (platform_index = 0; platform_index < platform_cnt; platform_index++) {
        err = clGetDeviceIDs( platforms[platform_index], \
                              CL_DEVICE_TYPE_ALL, 5, \
                              devices[platform_index], \
                              &device_cnt[platform_index] );

        if ((cnt + device_cnt[platform_index]) >= device_id) {
            platform_selected = platforms[platform_index];
            device_selected   = devices[platform_index][device_id - cnt - 1];
            opencl_available_status = 1;
            break;
        }

        cnt += device_cnt[platform_index];
    }

    if (opencl_available_status != 1) {
        fprintf(stderr, "Error: Invalid device id.\n");
        fprintf(stderr, "Info: Native code selected.\n");
        return 0;
    }

    err = clGetDeviceInfo(device_selected, CL_DEVICE_VENDOR, \
        sizeof(vendor), vendor ,NULL);
    err = clGetDeviceInfo(device_selected, CL_DEVICE_NAME,   \
        sizeof(name), name ,NULL);
    fprintf(stderr, "Info: Device %d %s %s (OpenCL) Selected.\n", \
        device_id, vendor, name);

    /* Create Content here inorder to simplify apis */
    context = clCreateContext(NULL, 1, &device_selected, NULL, NULL, &err);
    if (err != 0) {
        fprintf(stderr, "Error: Cannot Create OpenCL Content.\n");
        opencl_available_status = 0;
        return 0;
    }

    /* clCreateProgramWithSource Cannot use static memory, */
    /* dump static program into heap. */
    program_ptr = malloc((opencl_program_str_len + 1) * sizeof(char));
    memcpy(program_ptr, opencl_program_str, opencl_program_str_len + 1);

    program = clCreateProgramWithSource(context, 1, \
        (const char **)&program_ptr, &opencl_program_str_len, &err);
    if(err < 0) {
        fprintf(stderr, "Error: Couldn't create the program\n");
        opencl_available_status = 0;
        return 0;
    }

    err = clBuildProgram(program, \
        1, &device_selected, cl_compile_options, NULL, NULL);
    if(err < 0) {
        /* Get Compile log size */
        clGetProgramBuildInfo(program, device_selected, \
            CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        /* Alloc memory for Compile log */
        compile_log = malloc(log_size + 1);
        compile_log[log_size] = '\0';

        /* Read Compile log */
        clGetProgramBuildInfo(program, device_selected, \
            CL_PROGRAM_BUILD_LOG, log_size + 1, compile_log, NULL);

        fprintf(stderr, "%s\n", compile_log);
        free(compile_log);
        fprintf(stderr, "Error: OpenCL Program compile error.\n");
        opencl_available_status = 0;
        return 0;
    }

    do {

#if defined (CL_VERSION_2_0)

        queue = clCreateCommandQueueWithProperties(context, \
            device_selected, prop_default, &err);
        
        if (err == 0) {
            break;
        }

        fprintf(stderr, "Warning: Couldn't create a out of order command queue.\n");
        fprintf(stderr, "Info: Trying standard command queue.\n");
        
        queue = clCreateCommandQueueWithProperties(context, \
            device_selected, prop_inorder, &err);

        if (err == 0) {
            break;
        }

        fprintf(stderr, "Warning: Couldn't create a in order command queue.\n");
        fprintf(stderr, "Info: Trying host wide command queue.\n");
        
        queue = clCreateCommandQueueWithProperties(context, \
            device_selected, prop_host, &err);

        if (err < 0) {
            fprintf(stderr, "Error: Couldn't create OpenCL command queue.\n");
            opencl_available_status = 0;
            return 0;
        }

#else /* Code for OpenCL 1.0 1.1 1.2 */

        queue = clCreateCommandQueue(context, device_selected, \
            CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);

        if (err == 0) {
            break;
        }

        fprintf(stderr, "Warning: Couldn't create a out of order command queue.\n");
        fprintf(stderr, "Info: Trying standard command queue.\n");

        queue = clCreateCommandQueue(context, device_selected, 0, &err);

        if(err < 0) {
            fprintf(stderr, "Error: Couldn't create OpenCL command queue.\n");
            opencl_available_status = 0;
            return 0;
        }

#endif

    } while (0);

    fprintf(stderr, "Info: OpenCL Initialized on %s.\n", name);

    return 0;
}

cl_kernel opencl_load_kernel(char * program_name, int * err)
{
    return clCreateKernel(program, program_name, err);
}

int opencl_add_job(cl_kernel kernel, int dim, size_t* work_size)
{
    return clEnqueueNDRangeKernel(queue, kernel, dim, NULL, work_size, NULL, 0, NULL, NULL);
}

cl_mem opencl_create_rw_buffer(void * data, size_t size, int* err)
{
    return clCreateBuffer(context, CL_MEM_READ_WRITE|CL_MEM_COPY_HOST_PTR|CL_MEM_ALLOC_HOST_PTR, size, data, err);
}

cl_mem opencl_create_ro_buffer(void * data, size_t size, int* err)
{
    return clCreateBuffer(context, CL_MEM_READ_ONLY|CL_MEM_COPY_HOST_PTR, size, data, err);
}

int opencl_read_buffer(cl_mem buf, size_t size, void * data)
{
    clFinish(queue);
    return clEnqueueReadBuffer(queue, buf, CL_TRUE, 0, size, data, 0,NULL,NULL); 
}

void opencl_wait(void)
{
    clFinish(queue);
}
