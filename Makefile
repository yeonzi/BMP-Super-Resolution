CC     = clang
CFLAGS = -std=c99 -g -mfma -O3 -Wall -Wextra

all: bsr

bsr: bmp.o image.o image_resize.o image_conv.o image_merge.o main.c
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean run

clean:
	rm ./*.o
	rm ./bsr

run: bsr
	time ./bsr
