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

image_t * img_new(size_t width, size_t height, uint8_t model)
{
    image_t * img = NULL;

    do {
        if((img = malloc(sizeof(image_t))) == NULL ) break;

        img->model  = model;
        img->width  = width;
        img->height = height;
        img->data   = NULL;

        switch (model) {
            case IMG_MODEL_CIEXYZ : img->pixel_size = 3 * sizeof(int16_t); break;
            case IMG_MODEL_BGR    : img->pixel_size = 3 * sizeof(uint8_t); break;
            case IMG_MODEL_BGRA   : img->pixel_size = 4 * sizeof(uint8_t); break;
            case IMG_MODEL_YUV    : img->pixel_size = 3 * sizeof(int16_t); break;
            case IMG_MODEL_YCBCR  : img->pixel_size = 3 * sizeof(float);   break;
            case IMG_MODEL_HSV    : img->pixel_size = 3 * sizeof(int16_t); break;
            case IMG_MODEL_HSL    : img->pixel_size = 3 * sizeof(int16_t); break;
            case IMG_MODEL_CYMK   : img->pixel_size = 3 * sizeof(uint8_t); break;
        }

        img->data = malloc(img->width * img->height * img->pixel_size);
        if (img->data == NULL) break;

        return img;

    } while (0);

    if (img != NULL) free(img);
    return NULL;
}

void img_free(image_t * img)
{
    do {
        if (img == NULL) break;
        if (img->data != NULL) free(img->data);
    } while (0);
}

#define CYR     77    // 0.299
#define CYG     150    // 0.587
#define CYB      29    // 0.114

#define CUR     -43    // -0.16874
#define CUG    -85    // -0.33126
#define CUB     128    // 0.5

#define CVR      128   // 0.5
#define CVG     -107   // -0.41869
#define CVB      -21   // -0.08131

#define CSHIFT  8

int bgr_to_ycbcr(image_t * img)
{
    float B;
    float G;
    float R;
    float Y;
    float Cb;
    float Cr;
    
    uint8_t *ip;
    uint8_t *endp;
    float   *op;
    float   *wp;

    ip = img->data;
    endp = ip + 3 * img->width * img->height;

    op = malloc(3 * img->width * img->height * sizeof(float));
    wp = op;

    while (ip < endp) {
        B = ip[0];
        G = ip[1];
        R = ip[2];

        Y  =       + (0.299    * R) + (0.587    * G) + (0.114 * B);
        Cb = 128.0 - (0.168736 * R) - (0.331264 * G) + (0.5 * B);
        Cr = 128.0 + (0.5      * R) - (0.418688 * G) - (0.081312 * B);

        wp[0] = Y;
        wp[1] = Cb;
        wp[2] = Cr;

        ip += 3;
        wp += 3;
    }

    free(img->data);
    img->data  = op;
    img->model = IMG_MODEL_YCBCR;

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
    
    float   *ip;
    float   *endp;
    uint8_t *op;
    uint8_t *wp;

    ip = img->data;
    endp = ip + 3 * img->width * img->height;

    op = malloc(3 * img->width * img->height * sizeof(uint8_t));
    wp = op;

    while (ip < endp) {
        Y  = ip[0];
        Cb = ip[1];
        Cr = ip[2];

        R = Y +                            1.402 * (Cr - 128);
        G = Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128);
        B = Y +    1.772 * (Cb - 128);

        wp[0] = (uint8_t)round(B);
        wp[1] = (uint8_t)round(G);
        wp[2] = (uint8_t)round(R);

        ip += 3;
        wp += 3;
    }

    free(img->data);
    img->data  = op;
    img->model = IMG_MODEL_BGR;

    return 0;
}

int img_convert_to_ciexyz(image_t * img)
{
    return fprintf(stderr, "%s(): Not implemented\n", __FUNCTION__);
}

int img_convert_to_bgr(image_t * img)
{
    switch (img->model) {
        case IMG_MODEL_YCBCR: return ycbcr_to_bgr(img); break;
        default:
            fprintf(stderr, "%s(): Not implemented\n", __FUNCTION__);
            return -1;
    }

}

int img_convert_to_bgra(image_t * img)
{
    return fprintf(stderr, "%s(): Not implemented\n", __FUNCTION__);
}

int img_convert_to_yuv(image_t * img)
{
    return fprintf(stderr, "%s(): Not implemented\n", __FUNCTION__);
}

int img_convert_to_ycbcr(image_t * img)
{
    switch (img->model) {
        case IMG_MODEL_BGR: return bgr_to_ycbcr(img); break;
        default:
            fprintf(stderr, "%s(): Not implemented\n", __FUNCTION__);
            return -1;
    }
}


int img_convert_to_hsv(image_t * img)
{
    return fprintf(stderr, "%s(): Not implemented\n", __FUNCTION__);
}


int img_convert_to_hsl(image_t * img)
{
    return fprintf(stderr, "%s(): Not implemented\n", __FUNCTION__);
}

int img_convert_to_cymk(image_t * img)
{
    return fprintf(stderr, "%s(): Not implemented\n", __FUNCTION__);
}

int img_convert(image_t * img, uint8_t model)
{
    if (model == img->model) return 0;
    switch (model) {
        case IMG_MODEL_CIEXYZ : return img_convert_to_ciexyz(img); break;
        case IMG_MODEL_BGR    : return img_convert_to_bgr(img)   ; break;
        case IMG_MODEL_BGRA   : return img_convert_to_bgra(img)  ; break;
        case IMG_MODEL_YUV    : return img_convert_to_yuv(img)   ; break;
        case IMG_MODEL_YCBCR  : return img_convert_to_ycbcr(img) ; break;
        case IMG_MODEL_HSV    : return img_convert_to_hsv(img)   ; break;
        case IMG_MODEL_HSL    : return img_convert_to_hsl(img)   ; break;
        case IMG_MODEL_CYMK   : return img_convert_to_cymk(img)  ; break;
    }
}
