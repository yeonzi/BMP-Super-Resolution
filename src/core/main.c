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

#include <core/image_2x.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
	image_t * img1;
	image_t * img2;

	if (argc != 3) {
		fprintf(stderr, "Usage: %s [file1] [file2]\n", argv[0]);
		return -1;
	}

	img1 = image_open(argv[1]);

	if (img1 == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return -1;
	}

	img2 = image_2x(img1, INTERP_BASIC);

	return image_save(img2, argv[2], IMG_FMT_WINBMP);
}