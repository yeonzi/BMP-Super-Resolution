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

#include "image_resize.h"
#include <assert.h>
#include <stdio.h>
#include <math.h>

#if 0
int img_bilinear_int(image_t * img)
{
    uint8_t *img_ptr;
    uint8_t *img_end;

    uint8_t *img_upper_line;
    uint8_t *img_under_line;

    size_t  column;

    img_ptr = img->data;
    img_end = img_ptr + img->width * (img->height - 1) * img->pixel_size;

    column = 0;

    while (img_ptr < img_end) {
        img_ptr[3] = (img_ptr[0] + img_ptr[6]) / 2;
        img_ptr[4] = (img_ptr[1] + img_ptr[7]) / 2;
        img_ptr[5] = (img_ptr[2] + img_ptr[8]) / 2;

        img_ptr += 6;
        column  += 2;

        if (column == img->width) {
            column = 0;
            img_ptr += 3 * img->width;
        }
    }

    img_upper_line = img->data;
    img_ptr = img_upper_line + 3 * img->width;
    img_under_line = img_ptr + 3 * img->width;
    img_end = img_upper_line + img->width * img->height * img->pixel_size;

    column = 0;

    while (img_under_line < img_end) {
        img_ptr[0] = (img_upper_line[0] + img_under_line[0]) / 2;
        img_ptr[1] = (img_upper_line[1] + img_under_line[1]) / 2;
        img_ptr[2] = (img_upper_line[2] + img_under_line[2]) / 2;

        img_upper_line += 3;
        img_under_line += 3;
        img_ptr += 3;
        column  += 1;

        if (column == img->width) {
            column = 0;
            img_upper_line += 3 * img->width;
            img_ptr        += 3 * img->width;
            img_under_line += 3 * img->width;
        }
    }

    return 0;
}

image_t * img_2x(image_t * src)
{
    image_t * dst;

    uint8_t *src_ptr;
    uint8_t *src_end;
    uint8_t *dst_ptr;

    size_t column;

    image_convert(src, IMG_MODEL_BGR);
    src_ptr = src->data;
    src_end = src_ptr + src->width * src->height * src->pixel_size;

    dst = image_new(2 * src->width, 2 * src->height, IMG_MODEL_BGR);
    dst_ptr = dst->data;

    column = 0;

    while (src_ptr < src_end) {
        dst_ptr[0] = src_ptr[0];
        dst_ptr[1] = src_ptr[1];
        dst_ptr[2] = src_ptr[2];

        src_ptr += 3;
        dst_ptr += 6;
        column  ++;

        if (column == src->width) {
            column = 0;
            dst_ptr += 3 * dst->width;
        }
    }

    img_bilinear_int(dst);

    return dst;
}

#endif

image_t * img_resize_bicubic(image_t * img, int32_t newWidth, int32_t newHeight)
{
    image_t *out;
    int32_t *data_src;
    int32_t *data_dst;
    int32_t Cc;
    int32_t C[5];
    int32_t d0,d2,d3,a0,a1,a2,a3;
    int i,j,k,jj;
    int x,y;
    float dx,dy;
    float tx,ty;
    
    out = image_new(newWidth, newHeight, IMG_MODEL_BGR);
    data_src = img->data;
    data_dst = out->data;

    tx = (float)img->width / newWidth ;
    ty = (float)img->height / newHeight;

    fprintf(stderr, "Resize to (%d,%d) use bicubic interpolation\n", newWidth, newHeight);
    
    for(i = 0; i < newHeight; i++)
    for(j = 0; j < newWidth; j++) {

        x = (int)(tx * j);
        y = (int)(ty * i);
       
        dx = tx * j - x;
        dy = ty * i - y;

        for(k = 0; k < 3; k++) {
            for(jj = 0; jj <= 3; jj++) {
                              
                d0 = data_src[ (y - 1 + jj) * img->width * 3 + (x - 1)* 3 +k] - data_src[(y - 1 + jj) * img->width * 3 + (x) * 3 + k];
                d2 = data_src[ (y - 1 + jj) * img->width * 3 + (x + 1)* 3 +k] - data_src[(y - 1 + jj) * img->width * 3 + (x) * 3 + k];
                d3 = data_src[ (y - 1 + jj) * img->width * 3 + (x + 2)* 3 +k] - data_src[(y - 1 + jj) * img->width * 3 + (x) * 3 + k];
             
                a0 = data_src[(y - 1 + jj) * img->width * 3 + (x) * 3 + k];
                a1 = -1.0/3 * d0 + d2 -1.0/6 * d3;
                a2 =  1.0/2 * d0 + 1.0/2 * d2;
                a3 = -1.0/6 * d0 - 1.0/2 * d2 + 1.0/6 * d3;
                
                C[jj] = a0 + a1 * dx + a2 * dx * dx + a3 * dx * dx * dx;
            }

            d0 = C[0]-C[1];
            d2 = C[2]-C[1];
            d3 = C[3]-C[1];
         
            a0=C[1];
            a1 = -1.0/3 * d0 + d2 -1.0/6 * d3;
            a2 =  1.0/2 * d0 + 1.0/2 * d2;
            a3 = -1.0/6 * d0 - 1.0/2 * d2 + 1.0/6 * d3;
         
            Cc = a0 + a1 * dy + a2 * dy * dy + a3 * dy * dy * dy;

            data_dst[i * out->width * 3 + j * 3 + k] = Cc;
        }
       
    }
    return out;   
}



image_t * img_2x_bicubic(image_t * src)
{
    image_t * border  = NULL;
    image_t * scale   = NULL;
    image_t * chop    = NULL;

    float * src_ptr;
    float * src_end;
    float * dst_ptr;

    float *img_ptr;
    float *img_end;

    float *img_colum_0;
    float *img_colum_2;
    float *img_colum_4;
    float *img_colum_6;

    int B;
    int G;
    int R;

    int32_t column;
    int32_t line;

    image_convert(src, IMG_MODEL_BGR);

    border = image_make_border(src, 5);

    src_ptr = border->data;
    src_end = &src_ptr[3 * border->width * border->height];

    /* Resize */

    scale = image_new(2 * border->width, 2 * border->height, IMG_MODEL_BGR);
    dst_ptr = scale->data;

    column = 0;
    line = 0;

    while (src_ptr < src_end) {
        dst_ptr[0] = src_ptr[0];
        dst_ptr[1] = src_ptr[1];
        dst_ptr[2] = src_ptr[2];

        src_ptr = &src_ptr[3];
        dst_ptr = &dst_ptr[6];
        column  ++;

        if (column == border->width) {
            column = 0;
            dst_ptr = &dst_ptr[3 * scale->width];
        }
    }

    /* inter by line */

    img_ptr = scale->data;
    img_end = &((float *)scale->data)[3 * scale->width * (scale->height - 1)];

    line   = 0;
    column = 0;

    while (img_ptr < img_end) {
        B = lround(-0.0625 * (double)img_ptr[0] + 0.5625 * (double)img_ptr[6] + 0.5625 * (double)img_ptr[12] - 0.0625 * (double)img_ptr[18]);
        G = lround(-0.0625 * (double)img_ptr[1] + 0.5625 * (double)img_ptr[7] + 0.5625 * (double)img_ptr[13] - 0.0625 * (double)img_ptr[19]);
        R = lround(-0.0625 * (double)img_ptr[2] + 0.5625 * (double)img_ptr[8] + 0.5625 * (double)img_ptr[14] - 0.0625 * (double)img_ptr[20]);

        if (B < 0) B = 0;
        if (G < 0) G = 0;
        if (R < 0) R = 0;

        if (B > 255) B = 255;
        if (G > 255) G = 255;
        if (R > 255) R = 255;

        img_ptr[9]  = B;
        img_ptr[10] = G;
        img_ptr[11] = R;

        img_ptr = &img_ptr[6];
        column  += 2;

        if (column >= scale->width - 3) {
            column = 0;
            line += 2;
            img_ptr = &((float *)scale->data)[line * (3 * scale->width)];
        }
    }

    img_colum_0 = scale->data;
    img_colum_2 = &img_colum_0[3 * 2 * scale->width];
    img_colum_4 = &img_colum_0[3 * 4 * scale->width];
    img_colum_6 = &img_colum_0[3 * 6 * scale->width];
    
    img_ptr = &img_colum_0[3 * 3 * scale->width];

    img_end = &img_colum_0[3 * scale->width * (scale->height - 1)];

    line   = 0;
    column = 0;

    while (img_colum_6 < img_end) {
        B = lround(-0.0625 * (double)img_colum_0[0] + 0.5625 * (double)img_colum_2[0] + 0.5625 * (double)img_colum_4[0] - 0.0625 * (double)img_colum_6[0]);
        G = lround(-0.0625 * (double)img_colum_0[1] + 0.5625 * (double)img_colum_2[1] + 0.5625 * (double)img_colum_4[1] - 0.0625 * (double)img_colum_6[1]);
        R = lround(-0.0625 * (double)img_colum_0[2] + 0.5625 * (double)img_colum_2[2] + 0.5625 * (double)img_colum_4[2] - 0.0625 * (double)img_colum_6[2]);

        if (B < 0) B = 0;
        if (G < 0) G = 0;
        if (R < 0) R = 0;

        if (B > 255) B = 255;
        if (G > 255) G = 255;
        if (R > 255) R = 255;

        img_ptr[0] = B;
        img_ptr[1] = G;
        img_ptr[2] = R;

        img_colum_0 = &img_colum_0[3];
        img_colum_2 = &img_colum_2[3];
        img_colum_4 = &img_colum_4[3];
        img_colum_6 = &img_colum_6[3];
        img_ptr = &img_ptr[3];
        column  ++;

        if (column >= scale->width) {
            column = 0;
            line += 2;
            img_colum_0 = &((float *)scale->data)[3 * line * scale->width];
            img_colum_2 = &img_colum_0[3 * 2 * scale->width];
            img_colum_4 = &img_colum_0[3 * 4 * scale->width];
            img_colum_6 = &img_colum_0[3 * 6 * scale->width];
            
            img_ptr = &img_colum_0[3 * 3 * scale->width];
        }
    }

    chop = image_chop_border(scale, 10);

    return chop;
}
