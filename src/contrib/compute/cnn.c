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
#include <string.h>

/******************************************************************************
        Global Variable Area
    このプログラムはインスタンス化能力は持たない。
    二度目初期化なら、何が起こってもおかしくない。
******************************************************************************/

/* Plane buffer without memory alloate */
cl_mem * input_buffer_opencl;
cl_mem * output_buffer_opencl;

float ** input_buffer_native;
float ** output_buffer_native;

int run_with_opencl = 0;
int buffer_length;
int buffer_data_length;

int process_total;
int process_now;

int input_cnt_now;
int output_cnt_now;

int layer_id;


/******************************************************************************
        Private Function Declear Area
******************************************************************************/

int cnn_init_opencl_buffer(void);
int cnn_init_native_buffer(void);

int cnn_write_opencl_input_data(int buffer_id, float * data);
int cnn_write_native_input_data(int buffer_id, float * data);

float * cnn_read_opencl_input_data(int buffer_id);
float * cnn_read_native_input_data(int buffer_id);
float * cnn_read_opencl_output_data(int buffer_id);
float * cnn_read_native_output_data(int buffer_id);

int full_conv2d_layer_opencl( float * filters, float * bias, \
    int input_cnt, int output_cnt, int input_w, int input_h, \
    int filter_w, int filter_h, int dx, int dy );
int full_conv2d_layer_native( float * filters, float * bias, \
    int input_cnt, int output_cnt, int input_w, int input_h, \
    int filter_w, int filter_h, int dx, int dy );

int full_leaky_relu_layer_opencl(cl_mem * buffer, \
    float rate, int data_length, int plane_cnt);
int full_leaky_relu_layer_native(float ** buffer, \
    float rate, int data_length, int plane_cnt);

int cnn_switch_next_layer_opencl(void);
int cnn_switch_next_layer_native(void);


/******************************************************************************
        Public Function Definition Area
******************************************************************************/

int cnn_init(int buffer_cnt, int data_length)
{
    extern int run_with_opencl;
    extern int buffer_length;
    extern int buffer_data_length;
    extern int input_cnt_now;
    extern int output_cnt_now;
    extern int layer_id;

    buffer_length = buffer_cnt;
    buffer_data_length = data_length;
    input_cnt_now = 0;
    output_cnt_now = 0;
    layer_id = 0;

    run_with_opencl = opencl_available();

    if (run_with_opencl) {
        return cnn_init_opencl_buffer();
    } else {
        return cnn_init_native_buffer();
    }
    return 0;
}

int cnn_release(void)
{
    return 0;
}

int cnn_write_input_data(int buffer_id, float * data)
{
    extern int run_with_opencl;
    extern int buffer_length;

    if (buffer_id >= buffer_length) {
        return -1;
    }

    if (run_with_opencl) {
        return cnn_write_opencl_input_data(buffer_id, data);
    } else {
        return cnn_write_native_input_data(buffer_id, data);
    }
    return 0;
}

int cnn_push_input_data(float * data)
{
    extern int input_cnt_now;
    int ret;

    ret = cnn_write_input_data(input_cnt_now, data);

    if (ret == 0) {
        input_cnt_now ++;
    }

    return ret;
}

float * cnn_read_input_data(int buffer_id)
{
    extern int run_with_opencl;
    extern int buffer_length;

    if (buffer_id >= buffer_length) {
        return NULL;
    }

    if (run_with_opencl) {
        return cnn_read_opencl_input_data(buffer_id);
    } else {
        return cnn_read_native_input_data(buffer_id);
    }
    return NULL;
}

float * cnn_read_output_data(int buffer_id)
{
    extern int run_with_opencl;
    extern int buffer_length;

    if (buffer_id >= buffer_length) {
        return NULL;
    }

    if (run_with_opencl) {
        return cnn_read_opencl_output_data(buffer_id);
    } else {
        return cnn_read_native_output_data(buffer_id);
    }
    return NULL;
}

int cnn_full_conv2d_layer(float * filters, float * bias,        \
    int input_cnt, int output_cnt, int input_w, int input_h,    \
    int filter_w, int filter_h, int dx, int dy )
{
    extern int run_with_opencl;
    extern int input_cnt_now;
    extern int output_cnt_now;
    extern int layer_id;

    if ((input_cnt != input_cnt_now) && (input_cnt_now != 0)) {
        fprintf(stderr, "Warning: CNN layer incompatible. (Got %d, Except %d)\n", input_cnt, input_cnt_now);
    }
    
    input_cnt_now = input_cnt;
    output_cnt_now = output_cnt;
    layer_id ++;

    fprintf(stderr, "Running Full Convolution Layer %d ( %d => %d, "
        "Graph Size: %d x %d, Window Size: %d x %d)\n", \
        layer_id, input_cnt, output_cnt, input_w, input_h, filter_w, filter_h);

    if (run_with_opencl) {
        return full_conv2d_layer_opencl(filters, bias, input_cnt, output_cnt, \
            input_w, input_h, filter_w, filter_h, dx, dy );
    } else {
        return full_conv2d_layer_native(filters, bias, input_cnt, output_cnt, \
            input_w, input_h, filter_w, filter_h, dx, dy );
    }
    return 0;
}

int cnn_full_leaky_relu_layer(float rate)
{
    extern int run_with_opencl;
    extern cl_mem * output_buffer_opencl;
    extern float ** output_buffer_native;
    extern int buffer_data_length;
    extern int output_cnt_now;

    if (run_with_opencl) {
        return full_leaky_relu_layer_opencl( \
            output_buffer_opencl, rate, buffer_data_length, output_cnt_now);
    } else {
        return full_leaky_relu_layer_native( \
            output_buffer_native, rate, buffer_data_length, output_cnt_now);
    }

    return 0;
}

int cnn_switch_next_layer(void)
{
    extern int run_with_opencl;
    extern int input_cnt_now;
    extern int output_cnt_now;

    int ret;

    if (run_with_opencl) {
        ret = cnn_switch_next_layer_opencl();
    } else {
        ret = cnn_switch_next_layer_native();
    }

    input_cnt_now = output_cnt_now;
    output_cnt_now = 0;

    return ret;
}


/******************************************************************************

        OpenCL Private Function Definition Area

******************************************************************************/

int cnn_init_opencl_buffer()
{
    extern cl_mem * input_buffer_opencl;
    extern cl_mem * output_buffer_opencl;

    extern int buffer_length;

    input_buffer_opencl = malloc(buffer_length * sizeof(cl_mem));
    if (input_buffer_opencl == NULL) {
        perror("Cannot alloc memory for input buffers");
        return -1;
    }

    output_buffer_opencl = malloc(buffer_length * sizeof(cl_mem));
    if (input_buffer_opencl == NULL) {
        perror("Cannot alloc memory for output buffers");
        return -1;
    }

    return 0;
}

int cnn_write_opencl_input_data(int buffer_id, float * data)
{
    cl_int err;
    extern cl_mem * input_buffer_opencl;
    extern int buffer_data_length;

    input_buffer_opencl[buffer_id] = opencl_create_rw_buffer( \
        data, buffer_data_length * sizeof(float), &err);

    return err;
}

float * cnn_read_opencl_input_data(int buffer_id)
{
    float * out_buffer;
    extern cl_mem * input_buffer_opencl;
    extern int buffer_data_length;

    out_buffer = malloc(buffer_data_length * sizeof(float));
    if (out_buffer == NULL) {
        return NULL;
    }

    opencl_read_buffer(input_buffer_opencl[buffer_id], \
        buffer_data_length * sizeof(float), out_buffer);

    return out_buffer;
}

float * cnn_read_opencl_output_data(int buffer_id)
{
    float * out_buffer;
    extern cl_mem * output_buffer_opencl;
    extern int buffer_data_length;

    out_buffer = malloc(buffer_data_length * sizeof(float));
    if (out_buffer == NULL) {
        return NULL;
    }

    opencl_read_buffer(output_buffer_opencl[buffer_id], \
        buffer_data_length * sizeof(float), out_buffer);

    return out_buffer;
}

int full_conv2d_layer_opencl( float * filters, float * bias, \
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

    extern cl_mem * input_buffer_opencl;
    extern cl_mem * output_buffer_opencl;

    extern int process_total;
    extern int process_now;

    process_total = input_cnt * output_cnt;
    process_now   = 0;
    arr_size = input_w * input_h;
    filter_size = filter_w * filter_h;
    filter = filters;


    /* Create and alloc memory for bias array */
    bias_arr = malloc(arr_size * sizeof(float));

    /* set bias for each planc */
    for (output_index = 0; output_index < output_cnt; output_index++) {
        for (arr_index = 0; arr_index < arr_size; arr_index ++) {
            bias_arr[arr_index] = bias[output_index];
        }

        output_buffer_opencl[output_index] = opencl_create_rw_buffer( \
            bias_arr, arr_size * sizeof(float), &err);
    }

    free(bias_arr);

    for (output_index = 0; output_index < output_cnt; output_index++) {
        for (input_index = 0; input_index < input_cnt; input_index++) {
            conv2d_opencl(  input_buffer_opencl[input_index], \
                            output_buffer_opencl[output_index], \
                            input_w, input_h, \
                            filter, filter_w, filter_h, dx, dy);
            opencl_wait();
            process_now ++;
            filter = &filter[filter_size];
        }
    }

    return 0;
}

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

int full_leaky_relu_layer_opencl(cl_mem * buffer, \
    float rate, int data_length, int plane_cnt)
{
    int plane_index;

    for (plane_index = 0; plane_index < plane_cnt; plane_index ++) {
        leaky_relu_opencl(buffer[plane_index], rate, data_length);
    }

    return 0;
}

int cnn_switch_next_layer_opencl(void)
{
    extern int input_cnt_now;
    extern int output_cnt_now;
    extern cl_mem * input_buffer_opencl;
    extern cl_mem * output_buffer_opencl;
    int index;

    for (index = 0; index < input_cnt_now; index++) {
        clReleaseMemObject(input_buffer_opencl[index]);
    }

    for (index = 0; index < output_cnt_now; index++) {
        input_buffer_opencl[index] = output_buffer_opencl[index];
    }

    return 0;
}

/******************************************************************************

        Native Code Private Function Definition Area

******************************************************************************/

int cnn_init_native_buffer(void)
{
    extern float ** input_buffer_native;
    extern float ** output_buffer_native;

    extern int buffer_length;

    input_buffer_native = malloc(buffer_length * sizeof(float *));
    if (input_buffer_native == NULL) {
        perror("Cannot alloc memory for input buffers");
        return -1;
    }

    output_buffer_native = malloc(buffer_length * sizeof(float *));
    if (input_buffer_native == NULL) {
        perror("Cannot alloc memory for output buffers");
        return -1;
    }

    return 0;
}

int cnn_write_native_input_data(int buffer_id, float * data)
{
    extern float ** input_buffer_native;
    extern int buffer_data_length;

    input_buffer_native[buffer_id] = malloc(buffer_data_length * sizeof(float));
    if (input_buffer_native[buffer_id] == NULL) {
        return -1;
    }

    memcpy(input_buffer_native[buffer_id], data, buffer_data_length * sizeof(float));

    return 0;
}

float * cnn_read_native_input_data(int buffer_id)
{
    float * out_buffer;
    extern float ** input_buffer_native;
    extern int buffer_data_length;

    out_buffer = malloc(buffer_data_length * sizeof(float));
    if (out_buffer == NULL) {
        return NULL;
    }

    memcpy(out_buffer, \
        input_buffer_native[buffer_id], buffer_data_length * sizeof(float));

    return out_buffer;
}

float * cnn_read_native_output_data(int buffer_id)
{
    float * out_buffer;
    extern float ** output_buffer_native;
    extern int buffer_data_length;

    out_buffer = malloc(buffer_data_length * sizeof(float));
    if (out_buffer == NULL) {
        return NULL;
    }

    memcpy(out_buffer, \
        output_buffer_native[buffer_id], buffer_data_length * sizeof(float));

    return out_buffer;
}

int full_conv2d_layer_native( float * filters, float * bias, \
    int input_cnt, int output_cnt, int input_w, int input_h, \
    int filter_w, int filter_h, int dx, int dy )
{
    int input_index;
    int output_index;
    int arr_index;
    int arr_size;
    int filter_size;

    float * filter;

    extern float ** input_buffer_native;
    extern float ** output_buffer_native;

    extern int process_total;
    extern int process_now;

    process_total = input_cnt * output_cnt;
    process_now   = 0;
    arr_size = input_w * input_h;
    filter_size = filter_w * filter_h;
    filter = filters;

    /* set bias for each planc */
    for (output_index = 0; output_index < output_cnt; output_index++) {
        /* Create and alloc memory for bias array */
        output_buffer_native[output_index] = malloc(arr_size * sizeof(float));

        for (arr_index = 0; arr_index < arr_size; arr_index ++) {
            output_buffer_native[output_index][arr_index] = bias[output_index];
        }
    }

    for (output_index = 0; output_index < output_cnt; output_index++) {
        for (input_index = 0; input_index < input_cnt; input_index++) {
            fprintf(stderr, "\r %d of %d", process_now, process_total);
            fflush(stderr);
            conv2d_native(  input_buffer_native[input_index], \
                            output_buffer_native[output_index], \
                            input_w, input_h, \
                            filter, filter_w, filter_h, dx, dy);
            process_now ++;
            filter = &filter[filter_size];
        }
    }

    fprintf(stderr, "\r");
    fflush(stderr);

    return 0;
}

int full_leaky_relu_layer_native(float ** buffer, \
    float rate, int data_length, int plane_cnt)
{
    int plane_index;

    for (plane_index = 0; plane_index < plane_cnt; plane_index ++) {
        leaky_relu_native(buffer[plane_index], rate, data_length);
    }

    return 0;
}

int cnn_switch_next_layer_native(void)
{
    extern int input_cnt_now;
    extern int output_cnt_now;
    extern float ** input_buffer_native;
    extern float ** output_buffer_native;
    int index;

    for (index = 0; index < input_cnt_now; index++) {
        free(input_buffer_native[index]);
    }

    for (index = 0; index < output_cnt_now; index++) {
        input_buffer_native[index] = output_buffer_native[index];
    }

    return 0;
}

