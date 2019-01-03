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

int image_quarter(const char * input_file, const char * output_file)
{
	image_t * input_img;
	image_t * output_img;

	int width;
	int height;

	int ret;

	input_img = image_open(input_file);
	if (input_img == NULL) {
		perror("Cannot Open Input File.");
		return -1;
	}

	width = input_img->width / 2;
	height = input_img->height / 2;
	output_img = image_new(width, height, input_img->model);
	if (output_img == NULL) {
		image_free(input_img);
		perror("Cannot Create Output Object.");

		return -1;
	}

	if (input_img->model == IMG_MODEL_BGR) {
		for (width = 0; width < output_img->width; width ++) {
			for (height = 0; height < output_img->height; height ++) {
				image_pixel(output_img, width, height)[IMG_CHANNEL_B] = \
					image_pixel(input_img, width * 2, height * 2)[IMG_CHANNEL_B];
				image_pixel(output_img, width, height)[IMG_CHANNEL_G] = \
					image_pixel(input_img, width * 2, height * 2)[IMG_CHANNEL_G];
				image_pixel(output_img, width, height)[IMG_CHANNEL_R] = \
					image_pixel(input_img, width * 2, height * 2)[IMG_CHANNEL_R];
			}
		}
	} else {
		for (width = 0; width < output_img->width; width ++) {
			for (height = 0; height < output_img->height; height ++) {
				image_pixel(output_img, width, height)[IMG_CHANNEL_B] = \
					image_pixel(input_img, width * 2, height * 2)[IMG_CHANNEL_B];
				image_pixel(output_img, width, height)[IMG_CHANNEL_G] = \
					image_pixel(input_img, width * 2, height * 2)[IMG_CHANNEL_G];
				image_pixel(output_img, width, height)[IMG_CHANNEL_R] = \
					image_pixel(input_img, width * 2, height * 2)[IMG_CHANNEL_R];
				image_pixel(output_img, width, height)[IMG_CHANNEL_A] = \
					image_pixel(output_img, width * 2, height * 2)[IMG_CHANNEL_A];
			}
		}

	}

	image_free(input_img);

	ret = image_save(output_img, output_file, IMG_FMT_WINBMP);
	if (ret != 0) {
		perror("Cannot Create Output File.");
	}

	image_free(output_img);

	return ret;
}

int main(int argc, char const *argv[])
{
	if (argc != 3) {
		fprintf(stderr, "Usage: %s [input_file] [output_file]\n", argv[0]);
		return -1;
	}
	return image_quarter(argv[1],argv[2]);
}