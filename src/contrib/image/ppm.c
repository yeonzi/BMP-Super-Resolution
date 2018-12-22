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

#include "image.h"
#include <stdio.h>
#include <stdlib.h>

#define FILE_MAGIC_P1 0x3150
#define FILE_MAGIC_P2 0x3250
#define FILE_MAGIC_P3 0x3350
#define FILE_MAGIC_P4 0x3450
#define FILE_MAGIC_P5 0x3550
#define FILE_MAGIC_P6 0x3650

#define itoa(i,arr) {               \
do {                                \
    *(trans_buf_p) = i % 10 + '0';  \
    (trans_buf_p) ++;               \
    i = i / 10;                     \
} while (i > 0);                    \
while (trans_buf_p > trans_buf) {   \
    (trans_buf_p) --;               \
    *(arr) = *(trans_buf_p);        \
    (arr) ++;                       \
}}

image_t * ppm_p3_parse(char * ppm_file, size_t fsize)
{
    char * read_p;
    char * read_endp;

    int atoi_tmp;
    int is_num = 0;

    int width = 0;
    int height = 0;
    int depth;

    int x = 0;
    int y = 0;

    int read_cnt = 0;

    image_t * img = NULL;

    read_p    = ppm_file + 2;
    read_endp = ppm_file + fsize;

    while (read_p < read_endp) {
        
        /* ppm commit */
        if (*read_p == '#') {
            while (*read_p != '\r' && *read_p != '\n') {
                /* jump commit */
                read_p ++;
                if (read_p >= read_endp) {
                    if (img != NULL) {
                        free(img);
                    }
                    perror("Unexpected EOF");
                    return NULL;
                }
            }
        }

        /* is num */
        if (*read_p >= '0' && *read_p <= '9') {
            is_num = 1;
            atoi_tmp = atoi_tmp * 10 + (*read_p - '0');
        } else {
            if (is_num) {
                if (read_cnt == 0) {
                    width = atoi_tmp;
                } else if (read_cnt == 1) {
                    height = atoi_tmp;
                } else if (read_cnt == 2) {
                    depth = atoi_tmp;
                    if (depth != 255) {
                        perror("Not Standard PPM File. Stop.");
                        return NULL;
                    }
                    img = image_new(width, height, IMG_MODEL_BGR);
                    if (img == NULL) {
                        return NULL;
                    }
                } else {
                    if (read_cnt % 3 == 0) {
                        image_pixel(img, x, y)[IMG_CHANNEL_R] = atoi_tmp;
                    }else if (read_cnt % 3 == 1) {
                        image_pixel(img, x, y)[IMG_CHANNEL_G] = atoi_tmp;
                    }else if (read_cnt % 3 == 2) {
                        image_pixel(img, x, y)[IMG_CHANNEL_B] = atoi_tmp;
                        x ++;
                        if (x >= width) {
                            x = 0;
                            y ++;
                            if (y >= height) {
                                break;
                            }
                        }
                    }
                }

                is_num = 0;
                atoi_tmp = 0;
                read_cnt ++;
            }
        }
        
        read_p ++;
    }
    return img;
}

char * ppm_p3_pack(image_t * img, size_t *fsize)
{
    char header_template[] = "P3\n# Generate by YCIL\n";
    int header_length = 50;
    int pixel_cnt = 0;
    int mem_total = 0;

    int x,y,ch,pixel;
    
    char * file_buffer = NULL;
    char * file_buf_p = NULL;
    char * tran_buf_p = header_template;

    char trans_buf[10] = {0};
    char * trans_buf_p = trans_buf;

    pixel_cnt = img->width * img->height * 3;
    mem_total = pixel_cnt * 4 + header_length;

    file_buffer = malloc(mem_total * sizeof(char));
    file_buf_p = file_buffer;

    while (*tran_buf_p != '\0') {
        /* Write header */
        *file_buf_p = *tran_buf_p;
        file_buf_p ++;
        tran_buf_p ++;
    }

    x = img->width;
    itoa(x,file_buf_p);

    *file_buf_p = ' ';
    file_buf_p ++;

    y = img->height;
    itoa(y,file_buf_p);

    *file_buf_p = '\n';
    file_buf_p ++;

    ch = 255;
    itoa(ch,file_buf_p);

    *file_buf_p = '\n';
    file_buf_p ++;

    for (y = 0; y < img->height; y++) {
        for (x = 0; x < img->width; x++) {
            pixel = image_pixel(img, x, y)[IMG_CHANNEL_R];
            
            itoa(pixel,file_buf_p);

            *file_buf_p = ' ';
            file_buf_p ++;

            pixel = image_pixel(img, x, y)[IMG_CHANNEL_G];
            
            itoa(pixel,file_buf_p);

            *file_buf_p = ' ';
            file_buf_p ++;

            pixel = image_pixel(img, x, y)[IMG_CHANNEL_B];
            
            itoa(pixel,file_buf_p);

            *file_buf_p = ' ';
            file_buf_p ++;

            
        }
        *(file_buf_p - 1) = '\n';
    }

    *fsize = (file_buf_p - file_buffer) + 1;

    return file_buffer;
}


image_t * ppm_parse(char * ppm_file, size_t fsize)
{
    uint16_t magic;

    magic = *((short*)ppm_file);
    switch ( magic ) {
        case FILE_MAGIC_P3:
            return ppm_p3_parse(ppm_file, fsize);
            break;
        default:
            perror("Not Portable Pixmap Format File");
            return NULL;
    }
}


char * ppm_pack(image_t * img, size_t *fsize, short fmt)
{
    if (fmt == FILE_MAGIC_P3) {
        return ppm_p3_pack(img, fsize);
    } else {
        *fsize = 0;
    }
    return NULL;
}
