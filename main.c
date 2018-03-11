/********************************************************************************
The MIT License
Copyright (c) 2017 Yeonji
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
********************************************************************************/

#include "bmp.h"
#include "image.h"
#include "image_conv.h"
#include "image_merge.h"
#include "image_resize.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

void img_std(image_t * img, int channel)
{   
    float *ip;
    float *endp;

    ip = img->data;
    endp = &ip[3 * img->width * img->height - 1];

    ip = &ip[channel];
    while (ip < endp) {
        if(*ip < 0) *ip = 0.1 * *ip;
        ip = &ip[3];
    }
}

void img_std2(image_t * img, int channel, float max)
{   
    float ori_max;
    float k;
    float *ip;
    float *endp;

    ori_max = image_max(img, channel);
    k = max / ori_max;

    ip = img->data;
    endp = &ip[3 * img->width * img->height - 1];

    ip = &ip[channel];
    while (ip < endp) {
        *ip = k * *ip;
        ip = &ip[3];
    }
}

void img_ch_past(image_t * dst, image_t * src, int ch)
{ 
    float *ip;
    float *op;
    float *endp;

    ip = src->data;
    endp = &ip[3 * src->width * src->height - 3];
    op = dst->data;

    ip = &ip[ch];
    op = &op[ch];
    while (ip < endp) {
        *op = *ip;
        ip = &ip[3];
        op = &op[3];
    }
}

int main(int argc, const char ** argv)
{
    image_t * image     = NULL;
    image_t * image2x   = NULL;
    image_t * kernel    = NULL;
    image_t ** InputPlane;
    image_t ** OutputPlane;

    char filter[32];
    //char output[32];

    int i;
    int j;

    float max;

    InputPlane  = malloc(128 * sizeof(image_t*));
    OutputPlane = malloc(128 * sizeof(image_t*));

    if (argc <= 1) {
        image = bmp_load("test.bmp");
    } else {
        image = bmp_load(argv[1]);
    }
    image2x = img_2x_bicubic(image);

    image_free(image);

    max = image_max(image2x, 0);

    image = image_make_border(image2x, 3);

    image_free(image2x);

    InputPlane[0] = image;

    image_convert(InputPlane[0], IMG_MODEL_YCBCR);

    /******************************
        Round 1
    ******************************/

    for (i = 0; i < 32; i++) {
        fprintf(stderr, "\rRuning CNN Round 1 Output Plane %02d  ", i);

        sprintf(filter, "./filters/filter-1-%02d", i);
        kernel = kernel_load(filter);
        OutputPlane[i] = opencl_conv(InputPlane[0], kernel);
        image_free(kernel);
        fprintf(stderr, "Finished");
    }

    for (i = 0; i < 32; i++) {
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 1 Finished.\n");

    /******************************
        Round 2
    ******************************/

    for (i = 0; i < 32; i++) {
        for (j = 0; j < 32; j++) {
            fprintf(stderr, "\rRuning CNN Round 2 Output Plane %02d Input Plane %02d  ", i, j);
            
            sprintf(filter, "./filters/filter-2-%02d-%02d", i, j);
            kernel = kernel_load(filter);
            opencl_conv_and_merge(InputPlane[j], kernel);
            image_free(kernel);
        }
        OutputPlane[i] = opencl_get_conv_data();
        img_std(OutputPlane[i], 0);
    }

    for (i = 0; i < 32; i++) {
        image_free(InputPlane[i]);
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 2 Finished.\n");

    /******************************
        Round 3
    ******************************/

    for (i = 0; i < 32; i++) {
        fprintf(stderr, "\rRuning CNN Round 3 Output Plane %02d  ", i);
        sprintf(filter, "./filters/filter-3-%02d", i);
        kernel = kernel_load(filter);
        OutputPlane[i] = opencl_conv(InputPlane[i], kernel);
        image_free(kernel);
    }

    for (i = 0; i < 32; i++) {
        image_free(InputPlane[i]);
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 3 Finished.\n");

    /******************************
        Round 4
    ******************************/

    for (i = 0; i < 64; i++) {
        for (j = 0; j < 32; j++) {
            fprintf(stderr, "\rRuning CNN Round 4 Output Plane %02d Input Plane %02d  ", i, j);
            
            sprintf(filter, "./filters/filter-4-%02d-%02d", i, j);
            kernel = kernel_load(filter);
            opencl_conv_and_merge(InputPlane[j], kernel);
            image_free(kernel);
        }
        OutputPlane[i] = opencl_get_conv_data();
        img_std(OutputPlane[i], 0);
    }

    for (i = 0; i < 32; i++) {
        image_free(InputPlane[i]);
    }

    for (i = 0; i < 64; i++) {
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 4 Finished.\n");

    /******************************
        Round 5
    ******************************/

    for (i = 0; i < 64; i++) {
        fprintf(stderr, "\rRuning CNN Round 5 Output Plane %02d  ", i);
        sprintf(filter, "./filters/filter-5-%02d", i);
        kernel = kernel_load(filter);
        OutputPlane[i] = opencl_conv(InputPlane[i], kernel);
        image_free(kernel);
    }

    for (i = 0; i < 64; i++) {
        image_free(InputPlane[i]);
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 5 Finished.\n");

    /******************************
        Round 6
    ******************************/

    for (i = 0; i < 64; i++) {
        for (j = 0; j < 64; j++) {
            fprintf(stderr, "\rRuning CNN Round 6 Output Plane %02d Input Plane %02d  ", i, j);
            
            sprintf(filter, "./filters/filter-6-%02d-%02d", i, j);
            kernel = kernel_load(filter);
            opencl_conv_and_merge(InputPlane[j], kernel);
            image_free(kernel);
        }
        OutputPlane[i] = opencl_get_conv_data();
        img_std(OutputPlane[i], 0);
    }

    for (i = 0; i < 64; i++) {
        image_free(InputPlane[i]);
    }

    for (i = 0; i < 64; i++) {
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 6 Finished.\n");

    /******************************
        Round 7
    ******************************/

    for (i = 0; i < 64; i++) {
        fprintf(stderr, "\rRuning CNN Round 7 Output Plane %02d  ", i);
        sprintf(filter, "./filters/filter-7-%02d", i);
        kernel = kernel_load(filter);
        OutputPlane[i] = opencl_conv(InputPlane[i], kernel);
        image_free(kernel);
    }

    for (i = 0; i < 64; i++) {
        image_free(InputPlane[i]);
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 7 Finished.\n");

    /******************************
        Round 8
    ******************************/

    for (i = 0; i < 128; i++) {
        for (j = 0; j < 64; j++) {
            fprintf(stderr, "\rRuning CNN Round 8 Output Plane %02d Input Plane %02d  ", i, j);
            
            sprintf(filter, "./filters/filter-8-%02d-%02d", i, j);
            kernel = kernel_load(filter);
            opencl_conv_and_merge(InputPlane[j], kernel);
            image_free(kernel);
        }
        OutputPlane[i] = opencl_get_conv_data();
        img_std(OutputPlane[i], 0);
    }

    for (i = 0; i < 64; i++) {
        image_free(InputPlane[i]);
    }

    for (i = 0; i < 128; i++) {
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 8 Finished.\n");

    /******************************
        Round 9
    ******************************/

    for (i = 0; i < 128; i++) {
        fprintf(stderr, "\rRuning CNN Round 9 Output Plane %02d  ", i);
        sprintf(filter, "./filters/filter-9-%02d", i);
        kernel = kernel_load(filter);
        OutputPlane[i] = opencl_conv(InputPlane[i], kernel);
        image_free(kernel);
    }

    for (i = 0; i < 128; i++) {
        image_free(InputPlane[i]);
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 9 Finished.\n");

    /******************************
        Round 10
    ******************************/

    for (i = 0; i < 128; i++) {
        for (j = 0; j < 128; j++) {
            fprintf(stderr, "\rRuning CNN Round 10 Output Plane %02d Input Plane %02d  ", i, j);
            
            sprintf(filter, "./filters/filter-10-%02d-%02d", i, j);
            kernel = kernel_load(filter);
            opencl_conv_and_merge(InputPlane[j], kernel);
            image_free(kernel);
        }
        OutputPlane[i] = opencl_get_conv_data();
        img_std(OutputPlane[i], 0);
    }

    for (i = 0; i < 128; i++) {
        image_free(InputPlane[i]);
    }

    for (i = 0; i < 128; i++) {
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 10 Finished.                                \n");

    /******************************
        Round 11
    ******************************/

    for (i = 0; i < 128; i++) {
        fprintf(stderr, "\rRuning CNN Round 11 Output Plane %02d  ", i);
        sprintf(filter, "./filters/filter-11-%02d", i);
        kernel = kernel_load(filter);
        OutputPlane[i] = opencl_conv(InputPlane[i], kernel);
        image_free(kernel);
    }

    for (i = 0; i < 128; i++) {
        image_free(InputPlane[i]);
        InputPlane[i] = OutputPlane[i];
    }

    fprintf(stderr, "\r\033[KCNN Round 11 Finished.\n");

    /******************************
        Round 12
    ******************************/

    for (i = 0; i < 128; i++) {
        fprintf(stderr, "\rRuning CNN Round 12 Output Plane %02d  ", i);
        sprintf(filter, "./filters/filter-12-%02d", i);
        kernel = kernel_load(filter);
        opencl_conv_and_merge(InputPlane[i], kernel);
        image_free(kernel);
    }

    OutputPlane[0] = opencl_get_conv_data();

    fprintf(stderr, "\r\033[KCNN Round 12 Finished.\n");

    fprintf(stderr, "Rejust ...");

    img_std(OutputPlane[0], 0);
    img_std2(OutputPlane[0], 0, max);

    /* SIGSEGV */
    img_ch_past(image, OutputPlane[0], 0);

    fprintf(stderr, " Finished.\n");

    image_convert(image, IMG_MODEL_BGR);

    fprintf(stderr, "Save output image ...");

    bmp_save(image, "out.bmp");

    fprintf(stderr, "Finished.\a\n");

    return 0;
}
