#include <ycil/ycil.h>
#include <stdio.h>
#include "./src/file/bmp.h"

int main(int argc, char const *argv[])
{
	image_t * img;
	if(argc <= 1) {
		return -1;
	}
	img = image_open(argv[1]);
	bmp_save(img, "test_out.bmp");
	return 0;
}