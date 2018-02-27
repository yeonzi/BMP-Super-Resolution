/********************************************************************************
The MIT License
Copyright (c) 2018 Yeonji
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

#include "image.h"
#include "ppm.h"
#include <stdio.h>

image_t * ppm_load(const char *file_name)
{
	image_t * img;

	int  num;
	int  width;
	int  height;
	int  R;
	int  G;
	int  B;

	uint8_t * ptr;
	uint8_t * ptr_end;

	FILE * fp;

	fp = fopen(file_name, "rb");

	/* Read Magic */
	fscanf(fp, "P%d", &num);

	if (num != 3) return NULL;

	/* Read info */
	fscanf(fp, "%d %d", &width, &height);
	printf("%d x %d pixel\n", width, height);

	fscanf(fp, "%d", &num);
	if (num != 255) return NULL;

	img = img_new(width, height, IMG_MODEL_BGR);

	ptr = img->data;

	ptr_end = ptr + 3 * width * height;

	while (ptr < ptr_end) {
		fscanf(fp, "%d", &R);
		fscanf(fp, "%d", &G);
		fscanf(fp, "%d", &B);
		*ptr = B;
		ptr++;
		*ptr = G;
		ptr++;
		*ptr = R;
		ptr++;
	}

	return img;
}

