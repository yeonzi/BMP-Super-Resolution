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

#include "image_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define read_rgb_data(img,x,y) \
        B = (float)(image_pixel(img, x, y)[IMG_CHANNEL_B]); \
        G = (float)(image_pixel(img, x, y)[IMG_CHANNEL_G]); \
        R = (float)(image_pixel(img, x, y)[IMG_CHANNEL_R]); \

#define apply_rgb_data(img,x,y) \
        image_pixel(img, x, y)[IMG_CHANNEL_B] = lroundf(B); \
        image_pixel(img, x, y)[IMG_CHANNEL_G] = lroundf(G); \
        image_pixel(img, x, y)[IMG_CHANNEL_R] = lroundf(R); \

#define rgb_to_ycbcr() \
        Y  =       + (0.299    * R) + (0.587    * G) + (0.114    * B); \
        Cb = 128.0 - (0.168736 * R) - (0.331264 * G) + (0.5      * B); \
        Cr = 128.0 + (0.5      * R) - (0.418688 * G) - (0.081312 * B); \

#define ycbcr_to_rgb() \
        R = Y +                            1.402 * (Cr - 128); \
        G = Y - 0.344136 * (Cb - 128) - 0.714136 * (Cr - 128); \
        B = Y +    1.772 * (Cb - 128);                         \

float * image_extract_plane(image_t * img, int plane)
{
    float *data;
    int x,y;
    float B,G,R;
    float *dump_p;

    data = malloc(img->width * img->height * sizeof(float));
    if (data == NULL) {
        perror("Cannot Extract plane");
        return NULL;
    }

    dump_p = data;

    if (plane == IMAGE_PLANE_B) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                *dump_p = (float)(image_pixel(img, x, y)[IMG_CHANNEL_B]);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_G) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                *dump_p = (float)(image_pixel(img, x, y)[IMG_CHANNEL_G]);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_R) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                *dump_p = (float)(image_pixel(img, x, y)[IMG_CHANNEL_R]);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_Y) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                read_rgb_data(img,x,y);
                *dump_p = (0.299 * R) + (0.587 * G) + (0.114 * B);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_CB) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                read_rgb_data(img,x,y);
                *dump_p = 128.0 - (0.168736 * R) - (0.331264 * G) + (0.5 * B);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_CR) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                read_rgb_data(img,x,y);
                *dump_p = 128.0 + (0.5 * R) - (0.418688 * G) - (0.081312 * B);
                dump_p ++;
            }
        }
    }

    return data;
}

image_t * image_merge_plane(image_t * img, int plane, float * data)
{
    float * dump_p;
    float R,G,B,Y,Cb,Cr;

    int x,y;

    dump_p = data;

    if (plane == IMAGE_PLANE_B) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                image_pixel(img, x, y)[IMG_CHANNEL_B] = round(*dump_p);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_G) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                image_pixel(img, x, y)[IMG_CHANNEL_G] = round(*dump_p);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_R) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                image_pixel(img, x, y)[IMG_CHANNEL_R] = round(*dump_p);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_Y) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                read_rgb_data(img,x,y);
                rgb_to_ycbcr();
                Y = *dump_p;
                ycbcr_to_rgb();
                apply_rgb_data(img,x,y);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_CB) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                read_rgb_data(img,x,y);
                rgb_to_ycbcr();
                Cb = *dump_p;
                ycbcr_to_rgb();
                apply_rgb_data(img,x,y);
                dump_p ++;
            }
        }
    } else if (plane == IMAGE_PLANE_CR) {
        for (x = 0; x < img->width; x++) {
            for (y = 0; y < img->height; y++) {
                read_rgb_data(img,x,y);
                rgb_to_ycbcr();
                Cr = *dump_p;
                ycbcr_to_rgb();
                apply_rgb_data(img,x,y);
                dump_p ++;
            }
        }
    }

    return img;
}
