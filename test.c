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
#include <stdio.h>

int main(int argc, const char ** argv)
{
    image_t * image  = NULL;

    int cnt;
    int total;

    if (argc == 2) {
        total = atoi(argv[1]);
    } else {
        total = 1;
    }

    image = bmp_load("test.bmp");

    if (image == NULL){
        return -1;
    }

    for (cnt = 0; cnt < total; cnt++) {
        img_convert(image, IMG_MODEL_YCBCR);
        img_convert(image, IMG_MODEL_BGR);
    }

    bmp_save(image, "test_out.bmp");

    img_free(image);

    return 0;
}
