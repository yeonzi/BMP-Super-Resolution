#include "bmp.h"
#include "image.h"
#include "image_conv.h"

int main(void) {
	image_t * image;
	image_t * kernel;
	image_t * conv;

	image = bmp_load("./test.bmp");
	kernel = kernel_load("./test.kern");
	conv = image_conv(image, kernel);
	bmp_save(conv,"./out.bmp");
	return 0;
}