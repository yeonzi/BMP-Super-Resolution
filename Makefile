all:
	cc -std=c99 -g -O2 -Wall -Wextra test.c image.c bmp.c -o test
