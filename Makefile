# Make File for Yeonji Image Super Resolution

SRC_DIR=src
INC_DIR=${SRC_DIR}
OBJ_DIR=build
BIN_DIR=bin

CC?=gcc

CFLAGS+=-O2 -Wall -Wextra -ffunction-sections -fdata-sections
LDFLAGS?=-lOpenCL
MKDIR=mkdir -p

DIRS=bin build

CFLAGS+= -I${INC_DIR} -I/usr/local/include

all:bin/image_quarter bin/image_precision bin/image_2x

.PHONY:clean dirs

dirs:
	${MKDIR} $(DIRS)

clean:
	@rm -rvf ${OBJ_DIR}/*
	@rm -rvf ${BIN_DIR}/*

bin/image_quarter:${OBJ_DIR}/bmp.o ${OBJ_DIR}/ppm.o ${OBJ_DIR}/image_io.o ${OBJ_DIR}/image.o \
					${OBJ_DIR}/image_quarter.o
	${CC} ${CFLAGS} $^ -o $@

bin/image_precision:${OBJ_DIR}/bmp.o ${OBJ_DIR}/ppm.o ${OBJ_DIR}/image_io.o ${OBJ_DIR}/image.o \
					${OBJ_DIR}/image_precision.o
	${CC} ${CFLAGS} $^ -o $@

bin/image_2x:${OBJ_DIR}/bmp.o \
				${OBJ_DIR}/ppm.o \
				${OBJ_DIR}/image_io.o \
				${OBJ_DIR}/image.o \
				${OBJ_DIR}/image_2x.o \
				${OBJ_DIR}/image_plane.o \
				${OBJ_DIR}/image_border.o \
				${OBJ_DIR}/models.o \
				${OBJ_DIR}/vgg7yuv.o \
				${OBJ_DIR}/opencl.o \
				${OBJ_DIR}/conv2d.o \
				${OBJ_DIR}/relu.o \
				${OBJ_DIR}/cnn.o \
				${OBJ_DIR}/clprogram.o \
				${OBJ_DIR}/isr_main.o
	${CC} ${CFLAGS} $^ -o $@ ${LDFLAGS}

${OBJ_DIR}/bmp.o:src/contrib/image/bmp.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/ppm.o:src/contrib/image/ppm.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/image_io.o:src/contrib/image/image_io.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/image.o:src/contrib/image/image.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/isr_main.o:src/core/main.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/image_2x.o:src/core/image_2x.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/image_border.o:src/core/image_border.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/image_plane.o:src/core/image_plane.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/models.o:src/core/model.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/vgg7yuv.o:src/models/vgg7yuv.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/opencl.o:src/contrib/compute/opencl.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/conv2d.o:src/contrib/compute/conv2d.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/relu.o:src/contrib/compute/relu.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/cnn.o:src/contrib/compute/cnn.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/clprogram.o:${OBJ_DIR}/clprogram.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/clprogram.c:src/contrib/compute/clprogram.cl
	@echo "#include <stdlib.h>" > ${OBJ_DIR}/clprogram.c
	xxd --include src/contrib/compute/clprogram.cl | \
	sed 's/0x2a, 0x2f/0x2a, 0x2f, 0x0a, 0x00/g' | \
	sed 's/src_contrib_compute_clprogram_cl/opencl_program_str/g' | \
	sed 's/unsigned int/size_t/g' \
	>> ${OBJ_DIR}/clprogram.c

${OBJ_DIR}/image_quarter.o:src/utils/image_quarter.c
	${CC} -c ${CFLAGS} $^ -o $@

${OBJ_DIR}/image_precision.o:src/utils/image_precision.c
	${CC} -c ${CFLAGS} $^ -o $@
