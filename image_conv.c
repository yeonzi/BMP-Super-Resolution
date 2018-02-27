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

#include "image.h"
#include <stdio.h>

int32_t * image_rgb_pixel(image_t * image, uint32_t x, uint32_t y){
    return &((int32_t *)image->data)[3 * (y * image->width + x)];
}

float * image_ycbcr_pixel(image_t * image, uint32_t x, uint32_t y){
    return &((float *)image->data)[3 * (y * image->width + x)];
}

image_t * image_conv(image_t * src, image_t * kernel)
{
    image_t * border;
    image_t * conved;
    image_t * choped;

    size_t    border_size;
    int32_t   x0;
    int32_t   y0;
    int32_t   x1;
    int32_t   y1;
    int32_t   dx;
    int32_t   dy;
    int32_t   rgb_tmp[3];
    float   ycbcr_tmp[3];

    if (kernel->model != IMG_MODEL_BGR && kernel->model != IMG_MODEL_YCBCR) {
        fprintf(stderr, "RGB or YCbCr is the only color model supported by conv operation\n");
        return NULL;
    }

    if (kernel->width%2!=1||kernel->height%2!=1) {
        fprintf(stderr, "Conv need a odd kernel\n");
        return NULL;
    }

    dx = kernel->width / 2 + 1;
    dy = kernel->height / 2 + 1;

    border_size = kernel->width>kernel->height?kernel->width:kernel->height;

    border = img_make_border(src, border_size);

    img_convert(border, kernel->model);
    conved = img_new(border->width, border->height, kernel->model);

    if (kernel->model == IMG_MODEL_BGR) {
        int32_t * img;
        int32_t * kern;
        for(x0 = 0; x0 < border->width - kernel->width; x0++){
            for(y0 = 0; y0 < border->height - kernel->height; y0++){
                /* ergodic pixel in image */
                rgb_tmp[0] = 0;
                rgb_tmp[1] = 0;
                rgb_tmp[2] = 0;
                for(x1 = 0; x1 < kernel->width; x1++){
                    for(y1 = 0; y1 < kernel->height; y1++){
                        img  = image_rgb_pixel(border, x0+x1, y0+y1);
                        kern = image_rgb_pixel(kernel, x1, y1);
                        rgb_tmp[0] += kern[0] * img[0];
                        rgb_tmp[1] += kern[1] * img[1];
                        rgb_tmp[2] += kern[2] * img[2];
                    }
                }
                rgb_tmp[0] = rgb_tmp[0]/kernel->div + kernel->bias;
                rgb_tmp[1] = rgb_tmp[1]/kernel->div + kernel->bias;
                rgb_tmp[2] = rgb_tmp[2]/kernel->div + kernel->bias;

                img  = image_rgb_pixel(conved, x0+dx, y0+dy);
                img[0] = rgb_tmp[0];
                img[1] = rgb_tmp[1];
                img[2] = rgb_tmp[2];
                
            }
        }
    } else if (kernel->model == IMG_MODEL_YCBCR) {
        float * img;
        float * kern;
        for(x0 = 0; x0 < border->width - kernel->width; x0++){
            for(y0 = 0; y0 < border->height - kernel->height; y0++){
                /* ergodic pixel in image */
                ycbcr_tmp[0] = 0;
                ycbcr_tmp[1] = 0;
                ycbcr_tmp[2] = 0;
                for(x1 = 0; x1 < kernel->width; x1++){
                    for(y1 = 0; y1 < kernel->height; y1++){
                        img  = image_ycbcr_pixel(border, x0+x1, y0+y1);
                        kern = image_ycbcr_pixel(kernel, x1, y1);
                        ycbcr_tmp[0] += kern[0] * img[0];
                        ycbcr_tmp[1] += kern[1] * img[1];
                        ycbcr_tmp[2] += kern[2] * img[2];
                    }
                }
                ycbcr_tmp[0] = ycbcr_tmp[0] / kernel->div + kernel->bias;
                ycbcr_tmp[1] = ycbcr_tmp[1] / kernel->div + kernel->bias;
                ycbcr_tmp[2] = ycbcr_tmp[2] / kernel->div + kernel->bias;

                img  = image_ycbcr_pixel(conved, x0+dx, y0+dy);
                img[0] = ycbcr_tmp[0];
                img[1] = ycbcr_tmp[1];
                img[2] = ycbcr_tmp[2];
                
            }
        }
    }

    choped = img_chop_border(conved, border_size);

    img_free(border);

    return choped;
}

image_t * kernel_load(const char * file_name)
{
    image_t * img;

    int  num;
    int  width;
    int  height;

    int32_t * ptr;
    int32_t * ptr_end;

    float * Yptr;
    float * Yptr_end;

    FILE * fp;

    fp = fopen(file_name, "rb");

    /* Read Magic */
    fscanf(fp, "P%d", &num);

    if (num != 8) return NULL;

    /* Read info */
    fscanf(fp, "%d %d", &width, &height);

    fscanf(fp, "%d", &num);
    if (num == 255) {
        img = img_new(width, height, IMG_MODEL_BGR);
        ptr = img->data;
        ptr_end = &ptr[3 * width * height];
        fscanf(fp, "%f %f", &img->bias, &img->div);
        while (ptr < ptr_end) {
            fscanf(fp, "%d", ptr);
            ptr ++;
        }
    } else if (num == 256) {
        img = img_new(width, height, IMG_MODEL_YCBCR);
        Yptr = img->data;
        Yptr_end = &Yptr[3 * width * height];
        fscanf(fp, "%f %f", &img->bias, &img->div);
        while (Yptr < Yptr_end) {
            fscanf(fp, "%f", Yptr);
            Yptr ++;
        }
    } else {
        return NULL;
    }

    return img;
}
