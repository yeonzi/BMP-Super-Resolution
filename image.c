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
#include <stdlib.h>
#include <math.h>
#include <string.h>

image_t * image_new(int32_t width, int32_t height, uint8_t model)
{
    image_t * img = NULL;

    do {
        if((img = malloc(sizeof(image_t))) == NULL ) break;

        img->model  = model;
        img->width  = width;
        img->height = height;
        img->data   = NULL;

        switch (model) {
            /*
            case IMG_MODEL_CIEXYZ : img->pixel_size = 3 * sizeof(int16_t); break;
            case IMG_MODEL_BGR    : img->pixel_size = 3 * sizeof(uint8_t); break;
            case IMG_MODEL_BGRA   : img->pixel_size = 4 * sizeof(uint8_t); break;
            case IMG_MODEL_YUV    : img->pixel_size = 3 * sizeof(int16_t); break;
            case IMG_MODEL_YCBCR  : img->pixel_size = 3 * sizeof(float);   break;
            case IMG_MODEL_HSV    : img->pixel_size = 3 * sizeof(int16_t); break;
            case IMG_MODEL_HSL    : img->pixel_size = 3 * sizeof(int16_t); break;
            case IMG_MODEL_CYMK   : img->pixel_size = 3 * sizeof(uint8_t); break;
            */
            
            case IMG_MODEL_YCBCR  : img->pixel_size = 3 * sizeof(float);   break;
            case IMG_MODEL_BGR    : img->pixel_size = 3 * sizeof(float);   break;
        }

        img->data = malloc(img->width * img->height * img->pixel_size);
        if (img->data == NULL) break;

        memset(img->data, 0, img->width * img->height * img->pixel_size);

        return img;

    } while (0);

    if (img != NULL) free(img);
    return NULL;
}

void image_free(image_t * img)
{
    do {
        if (img == NULL) break;
        if (img->data != NULL) free(img->data);
        free(img);
    } while (0);
}

int bgr_to_ycbcr(image_t * img)
{
    float B;
    float G;
    float R;
    float Y;
    float Cb;
    float Cr;
    
    float *ip;
    float *endp;

    ip = img->data;
    endp = &ip[3 * img->width * img->height];

    while (ip < endp) {
        B = ip[0];
        G = ip[1];
        R = ip[2];

        Y  =       + (0.299    * R) + (0.587    * G) + (0.114    * B);
        Cb = 128.0 - (0.168736 * R) - (0.331264 * G) + (0.5      * B);
        Cr = 128.0 + (0.5      * R) - (0.418688 * G) - (0.081312 * B);

        ip[0] = Y;
        ip[1] = Cb;
        ip[2] = Cr;

        ip = &ip[3];
    }

    img->model = IMG_MODEL_YCBCR;

    fprintf(stderr, "\rImage was converted from BGR to YCbCr\n");

    return 0;
}

int ycbcr_to_bgr(image_t * img)
{
    float B;
    float G;
    float R;
    float Y;
    float Cb;
    float Cr;
    
    float *ip;
    float *endp;

    ip = img->data;
    endp = &ip[3 * img->width * img->height];

    while (ip < endp) {
        Y  = ip[0];
        Cb = ip[1];
        Cr = ip[2];

        R = Y +                            1.402 * (Cr - 128);
        G = Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128);
        B = Y +    1.772 * (Cb - 128);

        ip[0] = round(B);
        ip[1] = round(G);
        ip[2] = round(R);

        ip = &ip[3];
    }

    img->model = IMG_MODEL_BGR;

    fprintf(stderr, "\rImage was converted from YCbCr to BGR\n");

    return 0;
}

int image_convert(image_t * img, uint8_t model)
{
    if (model == img->model) return 0;
    switch (model) {
        case IMG_MODEL_CIEXYZ : break;
        case IMG_MODEL_BGR:
            switch (img->model) {
                case IMG_MODEL_YCBCR: return ycbcr_to_bgr(img); break;
                default: break;
            }
            break;
        case IMG_MODEL_BGRA   : break;
        case IMG_MODEL_YUV    : break;
        case IMG_MODEL_YCBCR:
            switch (img->model) {
                case IMG_MODEL_BGR: return bgr_to_ycbcr(img); break;
                default: break;
            }
            break;
        case IMG_MODEL_HSV    : break;
        case IMG_MODEL_HSL    : break;
        case IMG_MODEL_CYMK   : break;
    }

    return fprintf(stderr, "%s(): Not implemented\n", __FUNCTION__);
}

image_t * image_make_border(image_t * img, int32_t size)
{
    image_t * border;

    float   *src_ptr;
    float   *dst_ptr;

    int32_t  line;
    int32_t  column;

    border = image_new(img->width + 2 * size, img->height + 2 * size, img->model);

    /* copy image */
    for (line = 0; line < img->height; line++) {
        src_ptr = &((float*)img->data)[3 * line * img->width];
        dst_ptr = &((float*)border->data)[3 * ((line + size) * border->width + size)];
        for(column = 0; column < img->width; column++) {
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
        }
    }

    /* up */
    for (line = 0; line < size; line ++) {
        src_ptr = img->data;
        dst_ptr = &((float*)border->data)[3 * (line * border->width + size)];
        for(column = 0; column < img->width; column++) {
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
        }
    }
    
    /* down */
    for (line = 0; line < size; line ++) {
        src_ptr = &((float*)img->data)[3 * (img->height - 1) * img->width];
        dst_ptr = &((float*)border->data)[3 * ((border->height - line - 1) * border->width + size)];
        for(column = 0; column < img->width; column++) {
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
        }
    }

    /* left */
    for (line = 0; line < img->height; line ++) {
        dst_ptr = &((float*)border->data)[3 * border->width * (line + size)];
        for(column = 0; column < size; column++) {
            src_ptr = &((float*)img->data)[3 * line * img->width];
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
        }
    }

    /* right */
    for (line = 0; line < img->height; line ++) {
        dst_ptr = &((float*)border->data)[3 * (border->width * (line + size + 1) - size)];
        for(column = 0; column < size; column++) {
            src_ptr = &((float*)img->data)[3 * ((line + 1) * img->width - 1)];
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
        }
    }

    return border;
}

image_t * image_chop_border(image_t * img, int32_t size)
{
    image_t * chop;

    float   *src_ptr;
    float   *dst_ptr;

    int32_t  line;
    int32_t  column;

    chop = image_new(img->width - 2 * size, img->height - 2 * size, img->model);

    /* copy image */
    for (line = 0; line < chop->height; line++) {
        src_ptr = &((float*)img->data)[3 * ((line + size + 1) * img->width + size + 1)];
        dst_ptr = &((float*)chop->data)[3 * line * chop->width];
        for(column = 0; column < chop->width; column++) {
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
            *dst_ptr++ = *src_ptr++;
        }
    }

    return chop;
}

image_t * image_gray(image_t * img)
{
    float B;
    float G;
    float R;
    float Y;
    
    float *ip;
    float *endp;

    image_convert(img, IMG_MODEL_BGR);

    ip = img->data;
    endp = &ip[3 * img->width * img->height];

    while (ip < endp) {
        B = ip[0];
        G = ip[1];
        R = ip[2];

        Y = (0.299 * R) + (0.587 * G) + (0.114 * B);

        ip[0] = Y;
        ip[1] = Y;
        ip[2] = Y;

        ip = &ip[3];
    }

    return img;
}

float image_max(image_t * img, int channel)
{
    float max;
    
    float *ip;
    float *endp;

    ip = img->data;
    endp = &ip[3 * img->width * img->height];

    ip = &ip[channel];

    max = *ip;

    while (ip < endp) {
        if(*ip > max) max = *ip;
        ip = &ip[3];
    }

    return max;
}

float image_min(image_t * img, int channel)
{
    float min;
    
    float *ip;
    float *endp;

    ip = img->data;
    endp = &ip[3 * img->width * img->height];

    ip = &ip[channel];

    min = *ip;

    while (ip < endp) {
        if(*ip < min) min = *ip;
        ip = &ip[3];
    }

    return min;
}
