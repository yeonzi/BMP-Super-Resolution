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

#ifndef _ISR_IMAGE_H_
#define _ISR_IMAGE_H_ 1

#include <stdint.h>

/* Image structure */
typedef struct {
    int32_t     magic;      /* Should Always Be 0x54474D49   */
    int32_t     width;      /* Width of image by pixel       */
    int32_t     height;     /* Height of image by pixel      */
    int32_t     model;      /* Color model of this image     */
    int32_t     lStep;      /* Address step to next line     */
    int32_t     pStep;      /* Address step to next pixel    */
    uint8_t    *data;       /* Data area, Edit by APIs only. */
                            /* Data begins from upper left:  */
                            /* [ 0 , 1 , 2                   */
                            /*   3 , 4 , 5                   */
                            /*   6 , 7 , 8 ]                 */
} image_t;


#define IMG_FMT_WINBMP   0x4D42
#define IMG_FMT_ASCIIPPM 0x3350

/* Image magic number        */
/* This Magic Equ str 'IMGS' */
/* In convenient of debug    */
#define IMG_MAGIC           0x54474D49

/* Image coloe model */
#define IMG_MODEL_BGR    0
#define IMG_MODEL_BGRA   1

#define IMG_MODEL_RGB    IMG_MODEL_BGR
#define IMG_MODEL_RGBA   IMG_MODEL_BGRA


/* Image channel index for BGRA */
#define IMG_CHANNEL_B       0
#define IMG_CHANNEL_G       1
#define IMG_CHANNEL_R       2
#define IMG_CHANNEL_A       3

/* Image Instance manage APIs */
image_t * image_new(int32_t width, int32_t height, int32_t model);
void      image_free(image_t * img);

/* Pixel APIs */
uint8_t * image_pixel(image_t * img, int32_t x, int32_t y);

/* Image File IO apis */
image_t * image_open(const char * path);
int       image_save(image_t * img, const char * path, short fmt);

#endif