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

#ifndef YISR_COMPUTE_PRIVATE_H
#define YISR_COMPUTE_PRIVATE_H 1

#include <contrib/compute/compute.h>

#ifdef __APPLE__
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

cl_kernel opencl_load_kernel(char * program_name, int * err);

int opencl_add_job(cl_kernel kernel, int dim, size_t* work_size);

cl_mem opencl_create_rw_buffer(size_t size, int * err);
cl_mem opencl_create_ro_buffer(size_t size, int * err);

int opencl_read_buffer(cl_mem buf, size_t size, void * data);
int opencl_write_buffer(cl_mem buf, size_t offset, size_t size, void * data);

int opencl_mem_set(cl_mem mem, size_t size, float data);

void opencl_wait(void);

float * conv2d_native( float * input, float * output, int in_w, int in_h, \
                       float * filter, int k_w, int k_h, int dx, int dy );
int conv2d_opencl( cl_mem  input, cl_mem output, int in_w, int in_h, \
                    float * filter, int k_w, int k_h, int dx, int dy );

float * relu_native(float * buffer, int length);
int relu_opencl(cl_mem  buffer, int length);
float * leaky_relu_native(float * buffer, float rate, int length);
int leaky_relu_opencl(cl_mem  buffer, float rate, int length);

#endif