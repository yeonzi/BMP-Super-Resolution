CC     = clang
CFLAGS = -std=c99 -Os -Wall -Wextra

all: bsr

bsr: bmp.o image.o resize.o image_conv.o main.c
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

.PHONY:clean run

clean:
	rm ./*.o
	rm ./bsr

run: bsr
	time ./bsr
