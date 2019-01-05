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

#include <stdlib.h>
#include <stdio.h>

int full_convolution_layer_opencl( \
    cl_mem * input, cl_mem * output, float * filters, float * bias, \
    int input_cnt, int output_cnt, int input_w, int input_h,        \
    int filter_w, int filter_h, int dx, int dy )
{
    int input_index;
    int output_index;
    int arr_index;
    int arr_size;
    int filter_size;

    float * bias_arr;
    float * filter;

    cl_int err;

    arr_size = input_w * input_h;
    filter_size = filter_w * filter_h;
    filter = filters;

    fprintf(stderr, "Running Full Convolution Layer: %d => %d (Graph Size: %d x %d, Window Size: %d x %d)\n", \
        input_cnt, output_cnt, input_w, input_h, filter_w, filter_h);

    for (output_index = 0; output_index < output_cnt; output_index++) {
        /* Create and alloc */
        bias_arr = malloc(arr_size * sizeof(float));
        for (arr_index = 0; arr_index < arr_size; arr_index ++) {
            bias_arr[arr_index] = bias[output_index];
        }

        output[output_index] = opencl_create_rw_buffer( \
            bias_arr, arr_size * sizeof(float), &err);
    }

    for (output_index = 0; output_index < output_cnt; output_index++) {
        for (input_index = 0; input_index < input_cnt; input_index++) {
            conv2d_opencl(input[input_index], output[output_index], input_w, input_h, \
                        filter, filter_w, filter_h, dx, dy);
            opencl_wait();
            filter = &filter[filter_size];
        }
    }


    return 0;
}

int full_leaky_relu_layer_opencl(cl_mem * buffer, float rate, int data_length, int plane_cnt)
{
    int plane_index;

    for (plane_index = 0; plane_index < plane_cnt; plane_index ++) {
        leaky_relu_opencl(buffer[plane_index], rate, data_length);
    }

    return 0;
}
