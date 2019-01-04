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
#include <models/models.h>
#include <stdint.h>

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
    int32_t model_file_size;
    int64_t model_magic;
    int64_t model_ver;

    model_file_size = *(int32_t*)(model + 4);
    model_magic     = *(int64_t*)(model + 8);
    model_ver       = *(int64_t*)(model + 16);

    if (model_magic != MODEL_MAGIC) {
        return -1;
    }

    if (model_ver != MODEL_VER) {
        return -2;
    }

    if (model_file_size != (4 + 4 + 8 + 8 + 4 * 287585)) {
        return -3;
    }

    return 0;
}

image_t * vgg7_yuv_convert(image_t * origin, char * model)
{

}
