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
#include <stdlib.h>

image_t * image_make_border(image_t * img, int size)
{
	image_t * new_img;
	image_pixel_t src,dst;
	int x,y;

	new_img = image_new(img->width + 2*size, img->height + 2*size, img->model);

	if (new_img == NULL) {
		return NULL;
	}


    for (y = 0; y < img->height; y++) {
		for (x = 0; x < img->width; x++) {
			src = image_pixel(img, x, y);
			dst = image_pixel(new_img, x + size, y + size);
			dst[IMG_CHANNEL_B] = src[IMG_CHANNEL_B];
			dst[IMG_CHANNEL_G] = src[IMG_CHANNEL_G];
			dst[IMG_CHANNEL_R] = src[IMG_CHANNEL_R];
		}
	}
	return new_img;
}

image_t * image_chop_border(image_t * img, int size)
{
	image_t * new_img;
	image_pixel_t src,dst;
	int x,y;

	new_img = image_new(img->width - 2*size, img->height - 2*size, img->model);

	if (new_img == NULL) {
		return NULL;
	}

    for (y = 0; y < new_img->height; y++) {
		for (x = 0; x < new_img->width; x++) {
			src = image_pixel(img, x + size, y + size);
			dst = image_pixel(new_img, x, y);
			dst[IMG_CHANNEL_B] = src[IMG_CHANNEL_B];
			dst[IMG_CHANNEL_G] = src[IMG_CHANNEL_G];
			dst[IMG_CHANNEL_R] = src[IMG_CHANNEL_R];
		}
	}
	return new_img;
}
