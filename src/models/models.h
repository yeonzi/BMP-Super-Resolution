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

#ifndef YISR_MODEL_H
#define YISR_MODEL_H 1

#include <stdint.h>
#include <contrib/image/image.h>

char * model_read(const char *path);

typedef struct {
	int32_t model_file_magic;
    int32_t model_file_size;
    int32_t data_bias;
    int32_t data_length;
    int64_t model_magic;
    int64_t model_ver;
} model_header;

typedef struct {
    uint64_t    magic;
    char       *name;
    int       (*check)(const char*);
    image_t*  (*run)(image_t*, const char*);
} isr_model_t;

int vgg7_yuv_model_check(const char * model);
image_t * vgg7_yuv_convert(image_t * origin, const char * model);

#endif
