# Make File for Yeonji Image Super Resolution

CC:=gcc
CFLAGS:=-O2 -Wall -Wextra -Werror

CFLAGS=-O2 -Wall -Wextra
LDFLAGS:=
MKDIR:=mkdir -p

INCLUDE:=src
CFLAGS+= -I${INCLUDE}

SRCDIR:=src

all:bin/image_quarter bin/image_precision bin/image_2x

.PHONY:clean

clean:
	@rm -rvf obj/*
	@rm -rvf bin/*

bin/image_quarter:obj/bmp.o obj/ppm.o obj/image_io.o obj/image.o \
					obj/image_quarter.o
	${CC} ${CFLAGS} $^ -o $@

bin/image_precision:obj/bmp.o obj/ppm.o obj/image_io.o obj/image.o \
					obj/image_precision.o
	${CC} ${CFLAGS} $^ -o $@

bin/image_2x:obj/bmp.o obj/ppm.o obj/image_io.o obj/image.o \
				obj/image_2x.o obj/image_plane.o \
				obj/image_border.o obj/isr_main.o
	${CC} ${CFLAGS} $^ -o $@

obj/bmp.o:src/contrib/image/bmp.c
	${CC} -c ${CFLAGS} $< -o $@

obj/ppm.o:src/contrib/image/ppm.c
	${CC} -c ${CFLAGS} $< -o $@

obj/image_io.o:src/contrib/image/image_io.c
	${CC} -c ${CFLAGS} $< -o $@

obj/image.o:src/contrib/image/image.c
	${CC} -c ${CFLAGS} $< -o $@

obj/isr_main.o:src/core/main.c
	${CC} -c ${CFLAGS} $< -o $@

obj/image_2x.o:src/core/image_2x.c
	${CC} -c ${CFLAGS} $< -o $@

obj/image_border.o:src/core/image_border.c
	${CC} -c ${CFLAGS} $< -o $@

obj/image_plane.o:src/core/image_plane.c
	${CC} -c ${CFLAGS} $< -o $@

obj/image_quarter.o:src/utils/image_quarter.c
	${CC} -c ${CFLAGS} $< -o $@

obj/image_precision.o:src/utils/image_precision.c
	${CC} -c ${CFLAGS} $< -o $@
