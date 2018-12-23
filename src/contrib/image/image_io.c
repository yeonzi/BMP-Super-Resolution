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

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "image.h"
#include "ppm.h"
#include "bmp.h"

image_t * image_open(const char * path)
{
    struct stat statbuf;
    size_t file_size;
    FILE * file_p = NULL;
    char * file_buffer = NULL;
    int    block_cnt;
    short  magic;
    image_t * img = NULL;

    stat(path, &statbuf);
    file_size = statbuf.st_size;

    file_p = fopen(path, "rb");
    if (file_p == NULL) {
        perror("Cannot open file to read.");
        return NULL;
    }

    file_buffer = malloc(file_size * sizeof(char));
    if (file_buffer == NULL) {
        perror("Cannot alloc memory for file.");
        fclose(file_p);
        return NULL;
    }

    block_cnt = fread(file_buffer, 1, file_size, file_p);
    fclose(file_p);
    if (block_cnt == 0) {
        perror("Cannot read file.");
        free(file_buffer);
        return NULL;
    }

    magic = *((short*)file_buffer);
    switch ( magic ) {
        case IMG_FMT_WINBMP:
            img = bmp_parse(file_buffer, file_size);
            break;
        case IMG_FMT_ASCIIPPM:
            img = ppm_parse(file_buffer, file_size);
            break;
        default:
            fprintf(stderr, "Unknown Type.\n");
            break;
    }

    free(file_buffer);

    return img;
}

int image_save(image_t * img, const char * path, short fmt)
{
    char * file_buffer = NULL;
    size_t file_size   = 0;
    FILE * file_p = NULL;
    size_t block_cnt;

    if (fmt == IMG_FMT_WINBMP) {
        file_buffer = bmp_pack(img, &file_size);
    } else if (fmt == IMG_FMT_ASCIIPPM) {
        file_buffer = ppm_pack(img, &file_size, fmt);
    } else {
        fprintf(stderr, "Unknown Type.\n");
    }

    if (file_buffer == NULL) {
        perror("Cannot pack file");
        return -1;
    }

    file_p = fopen(path, "wb");
    if (file_p == NULL) {
        perror("Cannot open file for write.");
        return -1;
    }

    block_cnt = fwrite(file_buffer, 1, file_size, file_p);
    if (block_cnt < file_size) {
        perror("Write error, output file may broken.");
    }

    fclose(file_p);
    free(file_buffer);

    return 0;
}
