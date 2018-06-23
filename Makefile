# Makefile for YCIL

DIR_INC = ./include
DIR_SRC = ./src
DIR_BUILD = ./build
DIR_OBJ = ${DIR_BUILD}/obj
DIR_LIB = ${DIR_BUILD}/lib
SRC = $(wildcard ${DIR_SRC}/*.c)
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC}))

CC     = clang
CFLAGS = -std=c99 -mfma -O3 -Wall -Wextra -Werror -I${DIR_INC}

all: ${DIR_LIB}/libycil.so

${DIR_LIB}/libycil.so: \
		${DIR_OBJ}/image.o \
		${DIR_OBJ}/bmp.o \
		${DIR_OBJ}/ppm.o 
	$(CC) $(CFLAGS) -shared $^ -o $@

${DIR_OBJ}/image.o: ${DIR_SRC}/image/image.c  ${DIR_OBJ} ${DIR_LIB}
	$(CC) $(CFLAGS) -c $< -o $@

${DIR_OBJ}/bmp.o: ${DIR_SRC}/file/bmp.c  ${DIR_OBJ} ${DIR_LIB}
	$(CC) $(CFLAGS) -c $< -o $@

${DIR_OBJ}/ppm.o: ${DIR_SRC}/file/ppm.c  ${DIR_OBJ} ${DIR_LIB}
	$(CC) $(CFLAGS) -c $< -o $@

${DIR_OBJ}: ${DIR_BUILD}
	@mkdir -p $@

${DIR_LIB}: ${DIR_BUILD}
	@mkdir -p $@

${DIR_BUILD}:
	@mkdir -p $@

test: ./test.c ${DIR_LIB}/libycil.so
	$(CC) $(CFLAGS) -L./build/lib $< -o $@ -lycil

.PHONY:clean

clean:
	rm -rv ${DIR_BUILD}