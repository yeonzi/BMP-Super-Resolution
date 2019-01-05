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

float * conv2d_native( float * input,  int in_w, int in_h, \
                       float * filter, int k_w,  int k_h,  \
                       float bias, int dx, int dy )
{
    float * conved;
    int x0;
    int y0;
    int x1;
    int y1;
    int x_max;
    int y_max;
    float sum_tmp;

    conved = malloc(in_w * in_h * sizeof(float));
    if (conved == NULL) {
        perror("Cannot alloc memory for convolution.");
        return NULL;
    }

    x_max = in_w - k_w;
    y_max = in_h - k_h;

    for (x0 = 0; x0 < x_max; x0++) {
        for (y0 = 0; y0 < y_max; y0++) {
            sum_tmp = bias;
            for(x1 = 0; x1 < k_w; x1++) {
                for(y1 = 0; y1 < k_h; y1++) {
                    sum_tmp += \
                    input[in_w * (y0 + y1) + x0 + x1] * filter[k_w * y1 + x1];
                }
            }
            conved[in_w * (y0+dx) + x0 + dy] = sum_tmp;
        }
    }

    return conved;
}

int conv2d_opencl( cl_mem  input, cl_mem output, int in_w, int in_h, \
                    float * filter, int k_w, int k_h, int dx, int dy )
{
    cl_int          err;
    cl_kernel       kernel;
    cl_mem          filter_buf;
    size_t          work_size[2];

    work_size[0] = in_w - k_w;
    work_size[1] = in_h - k_h;

    kernel  = opencl_load_kernel("conv2d", &err);
    if(err < 0) {
        fprintf(stderr, "Cannot create OpenCL kernal object.\n");
        return -1;  
    }

    filter_buf = opencl_create_rw_buffer(filter, k_w*k_h*sizeof(float), &err);
    if(err < 0) {
        fprintf(stderr, "Cannot create OpenCL memory object. %d\n", err);
        return -1;  
    }

    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &filter_buf);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &in_w);
    err |= clSetKernelArg(kernel, 4, sizeof(int), &k_w);
    err |= clSetKernelArg(kernel, 5, sizeof(int), &k_h);
    err |= clSetKernelArg(kernel, 6, sizeof(int), &dx);
    err |= clSetKernelArg(kernel, 7, sizeof(int), &dy);
    if(err < 0) {
        fprintf(stderr, "Couldn't create a kernel argument\n");
        return -1;
    }

    err = opencl_add_job(kernel, 2, work_size);
    if(err < 0) {
        fprintf(stderr, "Couldn't enqueue the kernel\n");
        return -1;
    }

    clReleaseKernel(kernel);
    clReleaseMemObject(filter_buf);

    return 0;
}

float * conv2d( float * input,  int in_w, int in_h, \
                float * filter, int k_w,  int k_h,  \
                float bias, int dx, int dy )
{
    float * ret = NULL;
    int index, cnt;

    cl_mem input_buf;
    cl_mem output_buf;
    cl_int err;
    extern cl_command_queue    queue;
    if (opencl_available()) {
        cnt = in_w * in_h;

        /* Set Bias */
        ret = malloc((cnt + 1) * sizeof(float));
        for (index = 0; index < cnt; index++) {
            ret[index] = bias;
        }

        input_buf  = opencl_create_rw_buffer(input, cnt * sizeof(float), &err);
        if(err < 0) {
            fprintf(stderr, "Cannot create OpenCL memory object. %d\n", err);
        }
        output_buf = opencl_create_rw_buffer(ret, cnt * sizeof(float), &err);
        if(err < 0) {
            fprintf(stderr, "Cannot create OpenCL memory object. %d\n", err);
        }
        conv2d_opencl(input_buf, output_buf, in_w, in_h, \
                        filter, k_w, k_h, dx, dy);

        opencl_read_buffer(output_buf, cnt * sizeof(float), ret);

        clReleaseMemObject(input_buf);
        clReleaseMemObject(output_buf);

    } else {
        ret = conv2d_native(input, in_w, in_h, filter, k_w, k_h, bias, dx, dy);
    }
    return ret;
}
