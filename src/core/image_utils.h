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

#ifndef YISR_IMAGE_UTILS_H
#define YISR_IMAGE_UTILS_H 1

#include <contrib/image/image.h>

#define INTERP_NONE     0
#define INTERP_BASIC    1
#define INTERP_BILINER  2
#define INTERP_BICUBIC  3

/* Source: image_2x.c */
image_t * image_2x(image_t * src, int interp);

#define IMAGE_PLANE_B   0
#define IMAGE_PLANE_G   1
#define IMAGE_PLANE_R   2
#define IMAGE_PLANE_A   3
#define IMAGE_PLANE_Y   4
#define IMAGE_PLANE_CB  5
#define IMAGE_PLANE_CR  6

/* Note: This two functions may have bug when make change on YCbCr */
/* Source: image_plane.c */
float * image_extract_plane(image_t * img, int plane);
image_t * image_merge_plane(image_t * img, int plane, float * data);

/* Note: This two functions will free src image */
/* Source: image_border.c */
image_t * image_make_border(image_t * img, int size);
image_t * image_chop_border(image_t * img, int size);

#endif
