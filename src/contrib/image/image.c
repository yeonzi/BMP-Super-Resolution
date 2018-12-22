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

#include "image.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

image_t * image_new(int32_t width, int32_t height, int32_t model)
{
    image_t * img = NULL;

    img = malloc( sizeof( image_t ) ); 
    if( img == NULL ) {
        perror("Cannot alloc memory for new image object");
        return NULL;
    }

    img->magic  = IMG_MAGIC;
    img->width  = width;
    img->height = height;
    img->model  = model;
    img->data   = NULL;

    switch (img->model) {
        case IMG_MODEL_BGR:
            img->pStep = 3;
            break;
        case IMG_MODEL_BGRA:
            img->pStep = 4;
            break;
        default:
            img->pStep = 1;
    }

    img->lStep = img->pStep * img->width;
    img->data = malloc(img->width * img->height * img->pStep * sizeof(uint8_t));
    if ( img->data == NULL ) {
        perror("Cannot alloc memory for new image object");
        free(img);
        return NULL;
    }

    memset(img->data, 0, img->width * img->height * img->pStep * sizeof(uint8_t));

    return img;
}
void image_free(image_t * img)
{
    if (img == NULL) return;
    if (img->data != NULL) free(img->data);
    free(img);
}

uint8_t * image_pixel(image_t * img, int32_t x, int32_t y)
{
    return (img->data) + x * (img->pStep) + y * (img->lStep);
}
