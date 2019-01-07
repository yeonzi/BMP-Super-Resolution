# Make File for Yeonji Image Super Resolution

CC:=gcc
# CFLAGS:=-g -O2 -Wall -Wextra -Werror

CFLAGS=-O2 -mfma -Wall -Wextra -ffunction-sections -fdata-sections
LDFLAGS:=
MKDIR:=mkdir -p
BUILD_DIR=build
BIN_DIR=bin

DIRS=bin build

INCLUDE:=src
CFLAGS+= -I${INCLUDE}

SRCDIR:=src

all:bin/image_quarter bin/image_precision bin/image_2x

.PHONY:clean dirs

dirs:
	${MKDIR} $(DIRS)

clean:
	@rm -rvf ${BUILD_DIR}/*
	@rm -rvf bin/*

bin/image_quarter:${BUILD_DIR}/bmp.o ${BUILD_DIR}/ppm.o ${BUILD_DIR}/image_io.o ${BUILD_DIR}/image.o \
					${BUILD_DIR}/image_quarter.o
	${CC} ${CFLAGS} $^ -o $@

bin/image_precision:${BUILD_DIR}/bmp.o ${BUILD_DIR}/ppm.o ${BUILD_DIR}/image_io.o ${BUILD_DIR}/image.o \
					${BUILD_DIR}/image_precision.o
	${CC} ${CFLAGS} $^ -o $@

bin/image_2x:${BUILD_DIR}/bmp.o \
				${BUILD_DIR}/ppm.o \
				${BUILD_DIR}/image_io.o \
				${BUILD_DIR}/image.o \
				${BUILD_DIR}/image_2x.o \
				${BUILD_DIR}/image_plane.o \
				${BUILD_DIR}/image_border.o \
				${BUILD_DIR}/models.o \
				${BUILD_DIR}/vgg7yuv.o \
				${BUILD_DIR}/opencl.o \
				${BUILD_DIR}/conv2d.o \
				${BUILD_DIR}/relu.o \
				${BUILD_DIR}/cnn.o \
				${BUILD_DIR}/clprogram.o \
				${BUILD_DIR}/isr_main.o
	${CC} ${CFLAGS} $^ -o $@ -framework OpenCL

${BUILD_DIR}/bmp.o:src/contrib/image/bmp.c
	${CC} -c ${CFLAGS} $^ -o $@

${BUILD_DIR}/ppm.o:src/contrib/image/ppm.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/image_io.o:src/contrib/image/image_io.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/image.o:src/contrib/image/image.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/isr_main.o:src/core/main.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/image_2x.o:src/core/image_2x.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/image_border.o:src/core/image_border.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/image_plane.o:src/core/image_plane.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/models.o:src/core/model.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/vgg7yuv.o:src/models/vgg7yuv.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/opencl.o:src/contrib/compute/opencl.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/conv2d.o:src/contrib/compute/conv2d.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/relu.o:src/contrib/compute/relu.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/cnn.o:src/contrib/compute/cnn.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/clprogram.o:${BUILD_DIR}/clprogram.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/clprogram.c:src/contrib/compute/clprogram.cl
	@echo "#include <stdlib.h>" > ${BUILD_DIR}/clprogram.c
	xxd --include src/contrib/compute/clprogram.cl | \
	sed 's/0x2a, 0x2f/0x2a, 0x2f, 0x0a, 0x00/g' | \
	sed 's/src_contrib_compute_clprogram_cl/opencl_program_str/g' | \
	sed 's/unsigned int/size_t/g' \
	>> ${BUILD_DIR}/clprogram.c

${BUILD_DIR}/image_quarter.o:src/utils/image_quarter.c
	${CC} -c ${CFLAGS} $< -o $@

${BUILD_DIR}/image_precision.o:src/utils/image_precision.c
	${CC} -c ${CFLAGS} $< -o $@
