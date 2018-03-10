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


__kernel void conv(
    __global float * image,
    __global float * kern,
    __global float * image_out,
    int image_width,
    int kernel_width,
    int kernel_height,
    int dx,
    int dy,
    float bias,
    float div)
{
    int     x0;
    int     y0;
    int     x1;
    int     y1;
    float   CH0;
    float   CH1;
    float   CH2;

    x0 = get_global_id(0);
    y0 = get_global_id(1);

    CH0 = 0;
    CH1 = 0;
    CH2 = 0;
    for(x1 = 0; x1 < kernel_width; x1++) {
        for(y1 = 0; y1 < kernel_height; y1++) {
            CH0 += kern[3 * (y1 * kernel_width + x1) + 0] * image[ 3 * ((y0 + y1) * image_width + (x0 + x1)) + 0];
            CH1 += kern[3 * (y1 * kernel_width + x1) + 1] * image[ 3 * ((y0 + y1) * image_width + (x0 + x1)) + 1];
            CH2 += kern[3 * (y1 * kernel_width + x1) + 2] * image[ 3 * ((y0 + y1) * image_width + (x0 + x1)) + 2];
        }
    }
    image_out[ 3 * ((y0 + dy) * image_width + x0 + dx) + 0] = CH0 / div + bias;
    image_out[ 3 * ((y0 + dy) * image_width + x0 + dx) + 1] = CH1 / div + bias;
    image_out[ 3 * ((y0 + dy) * image_width + x0 + dx) + 2] = CH2 / div + bias;
}
