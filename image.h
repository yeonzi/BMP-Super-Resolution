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

#ifndef IMAGE_H
#define IMAGE_H 1

#include <stdint.h>
#include <stdlib.h>

#define IMG_MODEL_CIEXYZ    0
#define IMG_MODEL_BGR       1
#define IMG_MODEL_BGRA      2
#define IMG_MODEL_YUV       3
#define IMG_MODEL_YCBCR     4
#define IMG_MODEL_HSV       5
#define IMG_MODEL_HSL       6
#define IMG_MODEL_CYMK      7

typedef struct {
    uint8_t     model;
    size_t      width;
    size_t      height;
    size_t      pixel_size;
    void        *data;
} image_t;

image_t *   img_new(size_t width, size_t height, uint8_t model);
void        img_free(image_t * img);

int         img_convert(image_t * img, uint8_t model);


#endif /* IMAGE_H */
