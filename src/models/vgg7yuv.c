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
#include <contrib/compute/compute.h>
#include <contrib/compute/cnn.h>
#include <core/image_utils.h>
#include <core/model.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define MODEL_MAGIC 0x0056555937474756
#define MODEL_VER 0x0000006168706C41

char model_name[]="SRCNN_VGG7_Y";
int vgg7_yuv_model_check(char * model);
image_t * vgg7_yuv_convert(image_t * origin, char * model);

isr_model_t isr_model_vgg7yuv = {
    .magic = MODEL_MAGIC,
    .name  = model_name,
    .check = vgg7_yuv_model_check,
    .run   = vgg7_yuv_convert,
};


int vgg7_yuv_model_check(char * model)
{
    model_header * header;

    header = (model_header*)model;

    if (header->model_magic != MODEL_MAGIC) {
        return -1;
    }

    if (header->model_ver != MODEL_VER) {
        return -2;
    }

    if (header->data_length < (287585)) {
        return -3;
    }

    return 0;
}


image_t * vgg7_yuv_convert(image_t * origin, char * model)
{
    model_header * header;

    image_t * img;
    image_t * img_expand;
    float * data_ptr;
    float * layer_ptr;
    float * filters;
    float * biases;

    float * input_plane;
    float * output_plane;

    int in_cnt;
    int out_cnt;

    int img_w;
    int img_h;

    int img_index;
    float input_max;
    float output_max;
    float zoom_factor;

#if 0
    int layer_cnt = 7;
    int layer_index;
    int in_cnt[] =  {  1, 32, 32, 64,  64, 128, 128 };
    int out_cnt[] = { 32, 32, 64, 64, 128, 128,   1 };
#endif

    header = (model_header*)model;

    /* Do basic Image 2x */
    img = image_2x(origin, INTERP_BASIC);
    img_expand = image_make_border(img, 10);

    img_w = img_expand->width;
    img_h = img_expand->height;

    /* extract Y channel to convert */
    input_plane = image_extract_plane(img_expand, IMAGE_PLANE_Y);

    data_ptr = (float *)(&model[header->data_bias]);
    layer_ptr = data_ptr;

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

    cnn_init(128, img_w * img_h);

    cnn_push_input_data(input_plane);

    /* Layer 1 */

    in_cnt = 1;
    out_cnt = 32;
    filters = layer_ptr;
    biases = &layer_ptr[in_cnt * out_cnt * 9];

    cnn_full_conv2d_layer(filters, biases, in_cnt, out_cnt, img_w, img_h, 3, 3, 1, 1 );
    cnn_full_leaky_relu_layer(0.1);
    cnn_switch_next_layer();

    layer_ptr = &layer_ptr[in_cnt * out_cnt * 9 + out_cnt];

    
    /* Layer 2 */

    in_cnt = 32;
    out_cnt = 32;
    filters = layer_ptr;
    biases = &layer_ptr[in_cnt * out_cnt * 9];

    cnn_full_conv2d_layer(filters, biases, in_cnt, out_cnt, img_w, img_h, 3, 3, 1, 1 );
    cnn_full_leaky_relu_layer(0.1);
    cnn_switch_next_layer();

    layer_ptr = &layer_ptr[in_cnt * out_cnt * 9 + out_cnt];
    /* Layer 3 */

    in_cnt = 32;
    out_cnt = 64;
    filters = layer_ptr;
    biases = &layer_ptr[in_cnt * out_cnt * 9];

    cnn_full_conv2d_layer(filters, biases, in_cnt, out_cnt, img_w, img_h, 3, 3, 1, 1 );
    cnn_full_leaky_relu_layer(0.1);
    cnn_switch_next_layer();

    layer_ptr = &layer_ptr[in_cnt * out_cnt * 9 + out_cnt];
    /* Layer 4 */

    in_cnt = 64;
    out_cnt = 64;
    filters = layer_ptr;
    biases = &layer_ptr[in_cnt * out_cnt * 9];

    cnn_full_conv2d_layer(filters, biases, in_cnt, out_cnt, img_w, img_h, 3, 3, 1, 1 );
    cnn_full_leaky_relu_layer(0.1);
    cnn_switch_next_layer();

    layer_ptr = &layer_ptr[in_cnt * out_cnt * 9 + out_cnt];
    /* Layer 5 */

    in_cnt = 64;
    out_cnt = 128;
    filters = layer_ptr;
    biases = &layer_ptr[in_cnt * out_cnt * 9];

    cnn_full_conv2d_layer(filters, biases, in_cnt, out_cnt, img_w, img_h, 3, 3, 1, 1 );
    cnn_full_leaky_relu_layer(0.1);
    cnn_switch_next_layer();

    layer_ptr = &layer_ptr[in_cnt * out_cnt * 9 + out_cnt];
    /* Layer 6 */

    in_cnt = 128;
    out_cnt = 128;
    filters = layer_ptr;
    biases = &layer_ptr[in_cnt * out_cnt * 9];

    cnn_full_conv2d_layer(filters, biases, in_cnt, out_cnt, img_w, img_h, 3, 3, 1, 1 );
    cnn_full_leaky_relu_layer(0.1);
    cnn_switch_next_layer();

    layer_ptr = &layer_ptr[in_cnt * out_cnt * 9 + out_cnt];
    /* Layer 7 */

    in_cnt = 128;
    out_cnt = 1;
    filters = layer_ptr;
    biases = &layer_ptr[in_cnt * out_cnt * 9];

    cnn_full_conv2d_layer(filters, biases, in_cnt, out_cnt, img_w, img_h, 3, 3, 1, 1 );
    cnn_full_leaky_relu_layer(0.1);
    cnn_switch_next_layer();

    layer_ptr = &layer_ptr[in_cnt * out_cnt * 9 + out_cnt];

/******************************************************************************
    End of VGG Style SRCNN
******************************************************************************/

    fprintf(stderr, "Reading CNN Data.\n");

    output_plane = cnn_read_input_data(0);

    fprintf(stderr, "Doing Last Adjust.\n");

    output_max = 0.0;
    for (img_index = 0; img_index < img_w * img_h; img_index ++) {
        output_max += output_plane[img_index];
    }

    zoom_factor = input_max / output_max;
    fprintf(stderr, "Zoom Factor Detected: %f.\n", zoom_factor);

    for (img_index = 0; img_index < img_w * img_h; img_index ++) {
        output_plane[img_index] = output_plane[img_index] * zoom_factor;
    }

    fprintf(stderr, "Writing Output.\n");

    image_merge_plane(img_expand, IMAGE_PLANE_Y, output_plane);
    img = image_chop_border(img_expand, 10);

    return img;
}
