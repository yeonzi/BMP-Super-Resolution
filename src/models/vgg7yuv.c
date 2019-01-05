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

#include <contrib/image/image.h>
#include <contrib/compute/compute_private.h>
#include <contrib/compute/compute.h>
#include <contrib/compute/cnn.h>
#include <core/image_utils.h>
#include <models/models.h>
#include <stdint.h>
#include <stdio.h>

#define MODEL_MAGIC 0x0056555937474756
#define MODEL_VER 0x0000006168706C41

char model_name[]="SRCNN_VGG7_Y";
int vgg7_yuv_model_check(const char * model);
image_t * vgg7_yuv_convert(image_t * origin, const char * model);

isr_model_t isr_model_vgg7yuv = {
    .magic = MODEL_MAGIC,
    .name  = model_name,
    .check = vgg7_yuv_model_check,
    .run   = vgg7_yuv_convert,
};


int vgg7_yuv_model_check(const char * model)
{
    const model_header * header;

    header = (model_header*)model;

    if (header->model_magic != MODEL_MAGIC) {
        return -1;
    }

    if (header->model_ver != MODEL_VER) {
        return -2;
    }

    if (header->data_length < (4 * 287585)) {
        return -3;
    }

    return 0;
}

image_t * vgg7_yuv_convert(image_t * origin, const char * model)
{
    const model_header * header;

    image_t * img;
    image_t * img_expand;
    float * data_ptr;
    float * layer_ptr;
    float * filter_ptr;
    float * bias_ptr;

    float * input_plane;
    float * output_plane;

    int input_index;
    int output_index;
    int input_cnt;
    int output_cnt;

    int img_w;
    int img_h;


    int img_index;
    float input_max;
    float output_max;
    float zoom_factor;

    if (!opencl_available()) {
        fprintf(stderr, "VGG-7 SRCNN is not available for native code.\n");
        exit(-1);
    }

    header = (model_header*)model;

    cl_mem * in_buf;
    cl_mem * out_buf;

    in_buf = malloc(128 * sizeof(cl_mem));
    out_buf = malloc(128 * sizeof(cl_mem));

    /* Do basic Image 2x */
    img = image_2x(origin, INTERP_BASIC);
    img_expand = image_make_border(img, 10);

    image_save(img_expand, "./tmp/inner_00.bmp", IMG_FMT_WINBMP);

    img_w = img_expand->width;
    img_h = img_expand->height;

    /* extract Y channel to convert */
    input_plane = image_extract_plane(img_expand, IMAGE_PLANE_Y);
    output_plane = malloc(img_w * img_h * sizeof(float));
    img = image_new(img_expand->width, img_expand->height, img_expand->model);


    data_ptr = (float *)(&model[header->data_bias]);
    layer_ptr = data_ptr;

    in_buf[0] = opencl_create_rw_buffer(input_plane, img_w * img_h * sizeof(float), NULL);

    input_max = 0.0;
    for (img_index = 0; img_index < img_w * img_h; img_index ++) {
        input_max += input_plane[img_index];
    }

/******************************************************************************
    VGG Style SRCNN

Network Desctiption:
    Total Layer: 7
    Convert Plane: 1(Y)
    Layer1: 1 Plane => 32 Plane, 0.1 Leaky ReLU
    Layer2: 32 Plane => 32 Plane, 0.1 Leaky ReLU
    Layer3: 32 Plane => 64 Plane, 0.1 Leaky ReLU
    Layer4: 64 Plane => 64 Plane, 0.1 Leaky ReLU
    Layer5: 64 Plane => 128 Plane, 0.1 Leaky ReLU
    Layer6: 128 Plane => 128 Plane, 0.1 Leaky ReLU
    Layer7: 128 Plane => 1 Plane, 0.1 Leaky ReLU
    All Layer are using 3 * 3 kernel.

Data Count:
    data was saved by float.
    each layer need (input * output) kernels and (output) biases.
    layer size should be (input * output * kernel_length + output).
    total_size = 287585 * sizeof(float)

Model File Format:
    int32_t model_file_magic
    int32_t model_file_size
    int32_t data_bias
    int32_t data_length
    int64_t model_magic
    int64_t model_ver
    float   data[287585]

    data={
        layer[7]={
            output[m]={
                input[n]={
                    kernel[9]
                }
            },
            bias[m]
        }
    }
******************************************************************************/

    /* Layer 1 */

    input_cnt = 1;
    output_cnt = 32;
    filter_ptr = layer_ptr;
    bias_ptr = &layer_ptr[input_cnt * output_cnt * 9];

    full_convolution_layer_opencl(in_buf, out_buf, filter_ptr, bias_ptr, \
            input_cnt, output_cnt, img_w, img_h, 3, 3, 1, 1);
    full_leaky_relu_layer_opencl(out_buf, 0.1, (img_w * img_h), output_cnt);

    for (input_index = 0; input_index < input_cnt; input_index++) {
        clReleaseMemObject(in_buf[input_index]);
    }

    for (output_index = 0; output_index < output_cnt; output_index++) {
        in_buf[output_index] = out_buf[output_index];

    }

    layer_ptr = &layer_ptr[input_cnt * output_cnt * 9 + output_cnt];

    /* Layer 2 */

    input_cnt = 32;
    output_cnt = 32;
    filter_ptr = layer_ptr;
    bias_ptr = &layer_ptr[input_cnt * output_cnt * 9];

    full_convolution_layer_opencl(in_buf, out_buf, filter_ptr, bias_ptr, \
            input_cnt, output_cnt, img_w, img_h, 3, 3, 1, 1);
    full_leaky_relu_layer_opencl(out_buf, 0.1, (img_w * img_h), output_cnt);

    for (input_index = 0; input_index < input_cnt; input_index++) {
        clReleaseMemObject(in_buf[input_index]);
    }

    for (output_index = 0; output_index < output_cnt; output_index++) {
        in_buf[output_index] = out_buf[output_index];

    }

    layer_ptr = &layer_ptr[input_cnt * output_cnt * 9 + output_cnt];

    /* Layer 3 */

    input_cnt = 32;
    output_cnt = 64;
    filter_ptr = layer_ptr;
    bias_ptr = &layer_ptr[input_cnt * output_cnt * 9];

    full_convolution_layer_opencl(in_buf, out_buf, filter_ptr, bias_ptr, \
            input_cnt, output_cnt, img_w, img_h, 3, 3, 1, 1);
    full_leaky_relu_layer_opencl(out_buf, 0.1, (img_w * img_h), output_cnt);

    for (input_index = 0; input_index < input_cnt; input_index++) {
        clReleaseMemObject(in_buf[input_index]);
    }

    for (output_index = 0; output_index < output_cnt; output_index++) {
        in_buf[output_index] = out_buf[output_index];

    }

    layer_ptr = &layer_ptr[input_cnt * output_cnt * 9 + output_cnt];

    /* Layer 4 */

    input_cnt = 64;
    output_cnt = 64;
    filter_ptr = layer_ptr;
    bias_ptr = &layer_ptr[input_cnt * output_cnt * 9];

    full_convolution_layer_opencl(in_buf, out_buf, filter_ptr, bias_ptr, \
            input_cnt, output_cnt, img_w, img_h, 3, 3, 1, 1);
    full_leaky_relu_layer_opencl(out_buf, 0.1, (img_w * img_h), output_cnt);

    for (input_index = 0; input_index < input_cnt; input_index++) {
        clReleaseMemObject(in_buf[input_index]);
    }

    for (output_index = 0; output_index < output_cnt; output_index++) {
        in_buf[output_index] = out_buf[output_index];

    }

    layer_ptr = &layer_ptr[input_cnt * output_cnt * 9 + output_cnt];

    /* Layer 5 */

    input_cnt = 64;
    output_cnt = 128;
    filter_ptr = layer_ptr;
    bias_ptr = &layer_ptr[input_cnt * output_cnt * 9];

    full_convolution_layer_opencl(in_buf, out_buf, filter_ptr, bias_ptr, \
            input_cnt, output_cnt, img_w, img_h, 3, 3, 1, 1);
    full_leaky_relu_layer_opencl(out_buf, 0.1, (img_w * img_h), output_cnt);

    for (input_index = 0; input_index < input_cnt; input_index++) {
        clReleaseMemObject(in_buf[input_index]);
    }

    for (output_index = 0; output_index < output_cnt; output_index++) {
        in_buf[output_index] = out_buf[output_index];

    }

    layer_ptr = &layer_ptr[input_cnt * output_cnt * 9 + output_cnt];

    /* Layer 6 */

    input_cnt = 128;
    output_cnt = 128;
    filter_ptr = layer_ptr;
    bias_ptr = &layer_ptr[input_cnt * output_cnt * 9];

    full_convolution_layer_opencl(in_buf, out_buf, filter_ptr, bias_ptr, \
            input_cnt, output_cnt, img_w, img_h, 3, 3, 1, 1);
    full_leaky_relu_layer_opencl(out_buf, 0.1, (img_w * img_h), output_cnt);

    for (input_index = 0; input_index < input_cnt; input_index++) {
        clReleaseMemObject(in_buf[input_index]);
    }

    for (output_index = 0; output_index < output_cnt; output_index++) {
        in_buf[output_index] = out_buf[output_index];

    }

    layer_ptr = &layer_ptr[input_cnt * output_cnt * 9 + output_cnt];

    /* Layer 7 */

    input_cnt = 128;
    output_cnt = 1;
    filter_ptr = layer_ptr;
    bias_ptr = &layer_ptr[input_cnt * output_cnt * 9];

    full_convolution_layer_opencl(in_buf, out_buf, filter_ptr, bias_ptr, \
            input_cnt, output_cnt, img_w, img_h, 3, 3, 1, 1);
    full_leaky_relu_layer_opencl(out_buf, 0.1, (img_w * img_h), output_cnt);

    for (input_index = 0; input_index < input_cnt; input_index++) {
        clReleaseMemObject(in_buf[input_index]);
    }

    for (output_index = 0; output_index < output_cnt; output_index++) {
        in_buf[output_index] = out_buf[output_index];

    }

    layer_ptr = &layer_ptr[input_cnt * output_cnt * 9 + output_cnt];


/******************************************************************************
    End of VGG Style SRCNN
******************************************************************************/


    opencl_read_buffer(in_buf[0], img_w * img_h * sizeof(float), output_plane);

    output_max = 0.0;
    for (img_index = 0; img_index < img_w * img_h; img_index ++) {
        output_max += output_plane[img_index];
    }

    zoom_factor = input_max / output_max;

    for (img_index = 0; img_index < img_w * img_w; img_index ++) {
        output_plane[img_index] = output_plane[img_index] * zoom_factor;
    }

    image_merge_plane(img_expand, IMAGE_PLANE_Y, output_plane);
    img = image_chop_border(img_expand, 10);

    return img_expand;
}
