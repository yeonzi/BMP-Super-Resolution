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

#include "image_merge.h"
#include <stdio.h>

image_t * image_merge(image_t * src[], int32_t plane_cnt, int8_t method)
{
	image_t *output = NULL;
	int32_t  plane_index;
	
	int32_t  pixel_cnt;
	int32_t  pixel_index;

	output = image_new(src[0]->width, src[0]->height, src[0]->model);

	pixel_cnt = 3 * output->width * output->height;

	if (method == IMG_MERGE_ADD) {
		/* init output */
		for (pixel_index = 0; pixel_index < pixel_cnt; pixel_index ++) {
			((float*)output->data)[pixel_index] = ((float*)src[0]->data)[pixel_index];
		}

		for (plane_index = 1; plane_index < plane_cnt; plane_index++) {
			for (pixel_index = 0; pixel_index < pixel_cnt; pixel_index ++) {
				((float*)output->data)[pixel_index] += ((float*)(src[plane_index]->data))[pixel_index];
			}	
		}
	} else if (method == IMG_MERGE_AVERAGE) {
		/* init output */
		for (pixel_index = 0; pixel_index < pixel_cnt; pixel_index ++) {
			((float*)output->data)[pixel_index] = ((float*)src[0]->data)[pixel_index];
		}

		for (plane_index = 1; plane_index < plane_cnt; plane_index++) {
			for (pixel_index = 0; pixel_index < pixel_cnt; pixel_index ++) {
				((float*)output->data)[pixel_index] += ((float*)src[plane_index]->data)[pixel_index];
			}	
		}

		for (pixel_index = 0; pixel_index < pixel_cnt; pixel_index ++) {
			((float*)output->data)[pixel_index] = ((float*)output->data)[pixel_index] / plane_cnt;
		}
	}

	return output;
}
