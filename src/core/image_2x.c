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

void image_2x_interp_basic(image_t * img)
{
	int x, y;

	image_pixel_t src_p;
	image_pixel_t dst_p;

	for (x = 0; x < img->width; x+=2) {
		for (y = 0; y < img->height; y+=2) {
			src_p = image_pixel(img, x, y);
			dst_p = image_pixel(img, x + 1, y);
			dst_p[IMG_CHANNEL_B] = src_p[IMG_CHANNEL_B];
			dst_p[IMG_CHANNEL_G] = src_p[IMG_CHANNEL_G];
			dst_p[IMG_CHANNEL_R] = src_p[IMG_CHANNEL_R];

			dst_p = image_pixel(img, x, y + 1);
			dst_p[IMG_CHANNEL_B] = src_p[IMG_CHANNEL_B];
			dst_p[IMG_CHANNEL_G] = src_p[IMG_CHANNEL_G];
			dst_p[IMG_CHANNEL_R] = src_p[IMG_CHANNEL_R];

			dst_p = image_pixel(img, x + 1, y + 1);
			dst_p[IMG_CHANNEL_B] = src_p[IMG_CHANNEL_B];
			dst_p[IMG_CHANNEL_G] = src_p[IMG_CHANNEL_G];
			dst_p[IMG_CHANNEL_R] = src_p[IMG_CHANNEL_R];
		}
	}

}

image_t * image_2x(image_t * src, int interp)
{
	image_t * dst;

	int x, y;
	image_pixel_t src_p;
	image_pixel_t dst_p;

	dst = image_new(2 * src->width, 2 * src->height, IMG_MODEL_BGR);

	for (x = 0; x < src->width; x++) {
		for (y = 0; y < src->height; y++) {
			src_p = image_pixel(src, x, y);
			dst_p = image_pixel(dst, 2 * x, 2 * y);
			dst_p[IMG_CHANNEL_B] = src_p[IMG_CHANNEL_B];
			dst_p[IMG_CHANNEL_G] = src_p[IMG_CHANNEL_G];
			dst_p[IMG_CHANNEL_R] = src_p[IMG_CHANNEL_R];
		}
	}

	if (interp == INTERP_BASIC) {
		image_2x_interp_basic(dst);
	}

	return dst;
}
