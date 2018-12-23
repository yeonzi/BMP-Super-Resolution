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
#include <stdlib.h>
#include <stdio.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))

double image_precision(image_t * img1, image_t * img2)
{
	int width;
	int height;

	int x,y;
	int diff;

	long pixel_cnt;
	long total_square;
	double ave_error;

	if ( (img1->width != img2->width) || (img1->height != img2->height) ) {
		fprintf(stderr, "Image Size error, result may be error.");
	}

	width  = MIN(img1->width,  img2->width);
	height = MIN(img1->height, img2->height);

	pixel_cnt = width * height * 3;
	total_square = 0;

	for (x = 0; x < width; x++) {
		for (y = 0; y < height; y++) {
			diff = image_pixel(img1, x, y)[IMG_CHANNEL_B] - \
					image_pixel(img2, x, y)[IMG_CHANNEL_B];
			total_square += (diff * diff);
			diff = image_pixel(img1, x, y)[IMG_CHANNEL_G] - \
					image_pixel(img2, x, y)[IMG_CHANNEL_G];
			total_square += (diff * diff);
			diff = image_pixel(img1, x, y)[IMG_CHANNEL_R] - \
					image_pixel(img2, x, y)[IMG_CHANNEL_R];
			total_square += (diff * diff);
		}
	}

	ave_error = (double)total_square / (double) pixel_cnt;

	return ave_error;
}

int main(int argc, char const *argv[])
{
	image_t * img1;
	image_t * img2;

	img1 = image_open(argv[1]);
	img2 = image_open(argv[2]);

	if (img1 == NULL || img2 == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return -1;
	}

	printf("%3.8f\n", image_precision(img1, img2));

	return 0;
}
