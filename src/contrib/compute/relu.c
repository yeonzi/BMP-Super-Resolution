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

float * relu_native(float * buffer, int length)
{
    float * ptr;
    float * ptr_end;

    for (ptr = buffer, ptr_end = buffer + length ; ptr < ptr_end ; ptr++) {
        if (*ptr < 0) {
            *ptr = 0;
        }
    }

    return buffer;
}

float * leaky_relu_native(float * buffer, float rate, int length)
{
    float * ptr;
    float * ptr_end;

    for (ptr = buffer, ptr_end = buffer + length ; ptr < ptr_end ; ptr++) {
        if (*ptr < 0) {
            *ptr = *ptr * rate;
        }
    }

    return buffer;
}

int relu_opencl(cl_mem  buffer, int length)
{
    cl_int          err;
    cl_kernel       kernel;
    size_t          work_size;

    work_size = length;

    kernel  = opencl_load_kernel("relu", &err);
    if(err < 0) {
        fprintf(stderr, "Cannot create OpenCL kernal object.\n");
        return -1;  
    }

    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
    if(err < 0) {
        perror("Couldn't create a kernel argument");
        return -1;
    }

    err = opencl_add_job(kernel, 1, &work_size);
    if(err < 0) {
        perror("Couldn't enqueue the kernel");
        return -1;
    }

    clReleaseKernel(kernel);

    return 0;
}

int leaky_relu_opencl(cl_mem buffer, float rate, int length)
{
    cl_int          err;
    cl_kernel       kernel;
    size_t          work_size;

    work_size = length;

    kernel  = opencl_load_kernel("leaky_relu", &err);
    if(err < 0) {
        fprintf(stderr, "Cannot create OpenCL kernal object.%d\n", err);
        return -1;  
    }

    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer);
    err |= clSetKernelArg(kernel, 1, sizeof(float), &rate);
    if(err < 0) {
        perror("Couldn't create a kernel argument");
        return -1;
    }

    err = opencl_add_job(kernel, 1, &work_size);
    if(err < 0) {
        perror("Couldn't enqueue the kernel");
        return -1;
    }

    clReleaseKernel(kernel);

    return 0;
}

float * relu(float * input, int length)
{
    cl_mem buffer;
    cl_int err;

    if (opencl_available()) {

        buffer  = opencl_create_rw_buffer(length * sizeof(float), &err);
        opencl_write_buffer(buffer, 0, length * sizeof(float), input);
        
        relu_opencl(buffer, length);

        opencl_read_buffer(buffer, length * sizeof(float), input);

        clReleaseMemObject(buffer);

    } else {
        relu_native(input, length);
    }
    return input;
}

float * leaky_relu(float * input, float rate, int length)
{
    cl_mem buffer;
    cl_int err;

    if (opencl_available()) {

        buffer  = opencl_create_rw_buffer(length * sizeof(float), &err);
        opencl_write_buffer(buffer, 0, length * sizeof(float), input);
        
        leaky_relu_opencl(buffer, rate, length);

        opencl_read_buffer(buffer, length * sizeof(float), input);

        clReleaseMemObject(buffer);

    } else {
        leaky_relu_native(input, rate, length);
    }
    return input;
}
