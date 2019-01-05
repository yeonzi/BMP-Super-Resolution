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
        err = clGetDeviceIDs( platforms[platform_index], \
                              CL_DEVICE_TYPE_ALL, 5, \
                              devices[platform_index], \
                              &device_cnt[platform_index] );
    }

    for (platform_index = 0; platform_index < platform_cnt; platform_index++) {
        err = clGetPlatformInfo( platforms[platform_index], CL_PLATFORM_VENDOR,\
            sizeof(vendor), vendor ,NULL);
        err = clGetPlatformInfo( platforms[platform_index], CL_PLATFORM_NAME, \
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
            err = clGetDeviceInfo( devices[platform_index][device_index], \
                        CL_DEVICE_VENDOR, sizeof(vendor), vendor ,NULL);
            err = clGetDeviceInfo( devices[platform_index][device_index], \
                        CL_DEVICE_NAME, sizeof(name), name ,NULL);
            fprintf(stderr, "Device %d: %s %s (on OpenCL Platfrom %d)\n", \
                cnt, vendor, name, platform_index);
        }
    }

    return 0;
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

    unsigned int cnt = 0;

    extern int              opencl_available_status;
    extern cl_platform_id   platform_selected;
    extern cl_device_id     device_selected;
    extern cl_context       context;
    extern cl_program       program;
    extern cl_command_queue queue;

    char * program_ptr;
    // char cl_compile_options[] = "-cl-mad-enable -cl-fast-relaxed-math";
    char cl_compile_options[] = "-cl-mad-enable";
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

    queue = clCreateCommandQueue(context, device_selected, \
        CL_QUEUE_OUT_OF_ORDER_EXEC_MODE_ENABLE, &err);
    if(err < 0) {
        fprintf(stderr, "Warning: Couldn't create a out of order command queue.\n");
        fprintf(stderr, "Info: Trying standard command queue.\n");
        queue = clCreateCommandQueue(context, device_selected, 0, &err);
        if(err < 0) {
            fprintf(stderr, "Error: Couldn't create OpenCL command queue.\n");
            opencl_available_status = 0;
            return 0;
        }
    }

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
