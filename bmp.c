/********************************************************************************
The MIT License
Copyright (c) 2017 Yeonji
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
********************************************************************************/

#include "bmp.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#define uint16_LE(str)   (uint16_t)((uint16_t)(str[1] << 0) + (uint16_t)(str[0] << 8))

#define uint32_LE(str)   (uint32_t)((uint32_t)(str[3] << 0) +\
                                    (uint32_t)(str[2] << 8) +\
                                    (uint32_t)(str[1] << 16)+\
                                    (uint32_t)(str[0] << 24))

#define uint16_BE(str)   (uint16_t)((uint16_t)(str[0] << 0) + (uint16_t)(str[1] << 8))

#define uint32_BE(str)   (uint32_t)((uint32_t)(str[0] << 0) +\
                                    (uint32_t)(str[1] << 8) +\
                                    (uint32_t)(str[2] << 16)+\
                                    (uint32_t)(str[3] << 24))

#define str2_LE(str,content) {\
    str[1] = (uint8_t)((content&0x00FF) >> 0);\
    str[0] = (uint8_t)((content&0xFF00) >> 8);\
}
#define str4_LE(str,content) {\
    str[3] = (uint8_t)((content&0x000000FF) >> 0);\
    str[2] = (uint8_t)((content&0x0000FF00) >> 8);\
    str[1] = (uint8_t)((content&0x00FF0000) >> 16);\
    str[0] = (uint8_t)((content&0xFF000000) >> 24);\
}

#define str2_BE(str,content) {\
    str[0] = (uint8_t)((content&0x00FF) >> 0);\
    str[1] = (uint8_t)((content&0xFF00) >> 8);\
}
#define str4_BE(str,content) {\
    str[0] = (uint8_t)((content&0x000000FF) >> 0);\
    str[1] = (uint8_t)((content&0x0000FF00) >> 8);\
    str[2] = (uint8_t)((content&0x00FF0000) >> 16);\
    str[3] = (uint8_t)((content&0xFF000000) >> 24);\
}

#define s_free(ptr)  if(ptr!=NULL){free(ptr);};

/* BMP Header Struct */
typedef struct {
    unsigned char bfType[2];
    unsigned char bfSize[4];
    unsigned char bfReserved1[2];
    unsigned char bfReserved2[2];
    unsigned char bfOffBits[4];
}bmp_header_t;

/* Magic Number in BMP Header (bfType) */
#define WINDOWS_BMP         0X4D42  /*BM*/
#define OS2_BITMAP_ARRAY    0X4142  /*BA*/
#define OS2_COLOR_ICON      0X4943  /*CI*/
#define OS2_COLOR_POINTER   0X5043  /*CP*/
#define OS2_ICON            0X4349  /*IC*/
#define OS2_POINTER         0X5450  /*PT*/

/* DIB Header Struct */
typedef struct {
    uint32_t biSize;
    uint32_t biWidth;
    uint32_t biHeight;
    uint16_t biPlanes;
    uint16_t biBitCount;
    uint32_t biCompression;
    uint32_t biSizeImage;
    uint32_t biXPelsPerMeter;
    uint32_t biYPelsPerMeter;
    uint32_t biClrUsed;
    uint32_t biClrImportant;
}bmp_info_t;

#define DIB_BITMAPCOREHEADER_SIZE   12
#define DIB_OS22XBITMAPHEADER_SIZE  16
#define DIB_BITMAPINFOHEADER_SIZE   40
#define DIB_BITMAPV2INFOHEADER_SIZE 52
#define DIB_BITMAPV3INFOHEADER_SIZE 56
#define DIB_OS22XBITMAPHEADER2_SIZE 64
#define DIB_BITMAPV4HEADER_SIZE     108
#define DIB_BITMAPV5HEADER_SIZE     124

#define COMPRESS_BI_RGB             0
#define COMPRESS_BI_RLE8            1
#define COMPRESS_BI_RLE4            2
#define COMPRESS_BI_BITFIELDS       3
#define COMPRESS_BI_JPEG            4
#define COMPRESS_BI_PNG             5
#define COMPRESS_BI_ALPHABITFIELDS  6
#define COMPRESS_BI_CMYK            11
#define COMPRESS_BI_CMYKRLE8        12
#define COMPRESS_BI_CMYKRLE4        13

#define CHANNEL_R 0
#define CHANNEL_G 1
#define CHANNEL_B 2
#define CHANNEL_A 3

#define CHANNEL_CNT 4
#define CHANNEL_ALL 4

static const char * string_unknown           = "Unknown";

static const char * string_windows_bmp       = "Windows 3.1x bitmap (424D)";
static const char * string_os2_bitmap_array  = "OS/2 struct bitmap array (4241)";
static const char * string_os2_color_icon    = "OS/2 struct color icon (4349)";
static const char * string_os2_color_pointer = "OS/2 const color pointer (4350)";
static const char * string_os2_icon          = "OS/2 struct icon (4943)";
static const char * string_os2_pointer       = "OS/2 pointer (5054)";

static const char * string_bitmapcoreheader   = "BMP core header";
static const char * string_os22xbitmapheader  = "OS/2 BMP header";
static const char * string_bitmapinfoheader   = "Standard BMP header";
static const char * string_bitmapv2infoheader = "Adobe Photoshop Externed BMP header ver.2";
static const char * string_bitmapv3infoheader = "Adobe Photoshop Externed BMP header ver.3";
static const char * string_os22xbitmapheader2 = "OS/2 BMP header ver.2";
static const char * string_bitmapv4header     = "Standard BMP header ver.4";
static const char * string_bitmapv5header     = "Standard BMP header ver.5";

static const char * string_compress_bi_rgb             = "None";
static const char * string_compress_bi_rle8            = "RLE 8-bit/pixel";
static const char * string_compress_bi_rle4            = "RLE 4-bit/pixel";
static const char * string_compress_bi_bitfields       = "RGBA (Perhaps Huffman 1D)";
static const char * string_compress_bi_jpeg            = "JPEG image for printing";
static const char * string_compress_bi_png             = "PNG image for printing";
static const char * string_compress_bi_alphabitfields  = "RGBA bit field masks on Windows CE 5.0 with .NET 4.0 or later";
static const char * string_compress_bi_cmyk            = "Windows Metafile CMYK";
static const char * string_compress_bi_cmykrle8        = "Windows Metafile CMYK with RLE 8-bit/pixel";
static const char * string_compress_bi_cmykrle4        = "Windows Metafile CMYK with RLE 4-bit/pixel";

const char * get_bmp_type_string(const unsigned char * type_word)
{
    switch(uint16_BE(type_word))
    {
        case WINDOWS_BMP       : return string_windows_bmp       ; break;
        case OS2_BITMAP_ARRAY  : return string_os2_bitmap_array  ; break;
        case OS2_COLOR_ICON    : return string_os2_color_icon    ; break;
        case OS2_COLOR_POINTER : return string_os2_color_pointer ; break;
        case OS2_ICON          : return string_os2_icon          ; break;
        case OS2_POINTER       : return string_os2_pointer       ; break;
        default                : return string_unknown;
    };
    return string_unknown;
}

const char * get_bmp_dib_string(uint32_t size)
{
    switch(size)
    {
        case DIB_BITMAPCOREHEADER_SIZE   : return string_bitmapcoreheader   ; break;
        case DIB_OS22XBITMAPHEADER_SIZE  : return string_os22xbitmapheader  ; break;
        case DIB_BITMAPINFOHEADER_SIZE   : return string_bitmapinfoheader   ; break;
        case DIB_BITMAPV2INFOHEADER_SIZE : return string_bitmapv2infoheader ; break;
        case DIB_BITMAPV3INFOHEADER_SIZE : return string_bitmapv3infoheader ; break;
        case DIB_OS22XBITMAPHEADER2_SIZE : return string_os22xbitmapheader2 ; break;
        case DIB_BITMAPV4HEADER_SIZE     : return string_bitmapv4header     ; break;
        case DIB_BITMAPV5HEADER_SIZE     : return string_bitmapv5header     ; break;
        default                          : return string_unknown;
    }
    return string_unknown;
}

const char * get_bmp_comp_string(uint32_t method){
    switch(method)
    {
        case COMPRESS_BI_RGB            : return string_compress_bi_rgb            ; break;
        case COMPRESS_BI_RLE8           : return string_compress_bi_rle8           ; break;
        case COMPRESS_BI_RLE4           : return string_compress_bi_rle4           ; break;
        case COMPRESS_BI_BITFIELDS      : return string_compress_bi_bitfields      ; break;
        case COMPRESS_BI_JPEG           : return string_compress_bi_jpeg           ; break;
        case COMPRESS_BI_PNG            : return string_compress_bi_png            ; break;
        case COMPRESS_BI_ALPHABITFIELDS : return string_compress_bi_alphabitfields ; break;
        case COMPRESS_BI_CMYK           : return string_compress_bi_cmyk           ; break;
        case COMPRESS_BI_CMYKRLE8       : return string_compress_bi_cmykrle8       ; break;
        case COMPRESS_BI_CMYKRLE4       : return string_compress_bi_cmykrle4       ; break;
        default                         : return string_unknown;
    }
    return string_unknown;
}

typedef struct{
    FILE           *fp;
    image_t        *image;

    bmp_header_t    header;
    bmp_info_t      info;
    uint8_t        *raw_data;

    int32_t         data_offset;
    int32_t         color_depth;
    int32_t         width_fixed;
    int32_t         data_size;
} bmp_file_t;

/**************************************************

    BMP Data read && write

**************************************************/

int read_header(bmp_file_t *bmp)
{
    fseek(bmp->fp, 0, SEEK_SET);

    /* read bmp header */
    if(fread(&bmp->header, sizeof(bmp->header), 1, bmp->fp) != 1){
        fprintf(stderr, "Broken File: Can not read header.\n");
        return -1;
    }

    /* unsupported bmp format */
    if(uint16_BE(bmp->header.bfType) != WINDOWS_BMP){
        fprintf(stderr, "Unsupported Format: %s.\n", get_bmp_type_string(bmp->header.bfType));
        return -1;
    } else {
        fprintf(stderr, "Format: %s.\n", get_bmp_type_string(bmp->header.bfType));
    }

    /* transform to uint32_t */
    bmp->data_offset = uint32_BE(bmp->header.bfOffBits);

    return 0;
}

int read_info(bmp_file_t *bmp)
{
    fseek(bmp->fp, 14, SEEK_SET);

    /* get bmp information header */
    fread(&bmp->info, sizeof(bmp->info), 1, bmp->fp);

    /* unsupported bmp format */
    if(bmp->info.biSize < DIB_BITMAPINFOHEADER_SIZE){
        fprintf(stderr, "Unsupported DIB Format: %s.\n", \
                get_bmp_dib_string(bmp->info.biSize));
        return -1;
    }
    fprintf(stderr, "DIB Format: %s.\n", get_bmp_dib_string(bmp->info.biSize));

    /* set bytes_per_pixel and read image */
    if(bmp->info.biBitCount == 16){
        /* 16-bit RGB use two byte every pixel */
        bmp->color_depth=2;
    }else if(bmp->info.biBitCount == 24){
        /* 24-bit RGB use three byte every pixel */
        bmp->color_depth=3;
    }else if(bmp->info.biBitCount == 32){
        /* 32-bit RGB use four byte every pixel */
        bmp->color_depth=4;
    }else{
        /* unsupported number of bits per pixel */
        fprintf(stderr, "Unsupported Format: Can not parse %d-bit color.\n", \
                bmp->info.biBitCount);
        return -1;
    }

    /* unsupported compression method */
    if(bmp->info.biCompression != COMPRESS_BI_RGB &&\
       bmp->info.biCompression != COMPRESS_BI_BITFIELDS){
        fprintf(stderr, "Unsupported Format: Can not parse %s compression method.\n", \
                get_bmp_comp_string(bmp->info.biCompression));
        return -1;
    }

    /* unsupported compression method */
    /* COMPRESS_BI_BITFIELDS marked, but color depth is not 4 */
    /* this will appear when use Huffman 1D */
    if(bmp->info.biCompression == COMPRESS_BI_BITFIELDS && bmp->color_depth != 4){
        fprintf(stderr, "Unsupported Format: Can not parse with Huffman 1D.\n");
        return -1;
    }

    return 0;
}

int read_data(bmp_file_t *bmp)
{
    size_t data_size;

    bmp->width_fixed = bmp->info.biWidth * bmp->color_depth;

    if (bmp->width_fixed % 4 != 0) {
        bmp->width_fixed = (bmp->width_fixed - bmp->width_fixed % 4 + 4);
    }

    /* get data size in theory */
    data_size = bmp->width_fixed * bmp->info.biHeight;

    /* locate file stream */
    if(fseek(bmp->fp, bmp->data_offset, SEEK_SET) != 0){
        fprintf(stderr, "Error: Can not locate file stream.\n");
        return -1;
    }

    bmp->raw_data = malloc((data_size) * sizeof(uint8_t));
    
    if(bmp->raw_data == NULL){
        fprintf(stderr, "Error: Can not malloc for new object.\n");
        return -1;
    }

    /* read bmp data */
    if(fread(bmp->raw_data, data_size, 1, bmp->fp) != 1){
        fprintf(stderr, "Broken File: Can not read data.\n");
        return -1;
    }

    return 0;
}

/**************************************************

    BMP Data Encode && Decode

**************************************************/

int decode_data_24(bmp_file_t *bmp)
{
    int32_t line    = bmp->info.biHeight;
    int32_t column  = 0;

    uint8_t *src;
    float   *dst = bmp->image->data;

    int32_t width;

    width = 3 * bmp->info.biWidth;

    for (line--; line >= 0; line--) {
        src = bmp->raw_data + line * bmp->width_fixed;
        for(column = 0; column < width; column++) {
            *dst = *src;
            src ++;
            dst ++;
        }
    }

    return 0;
}

int decode_data(bmp_file_t *bmp)
{
    switch (bmp->color_depth) {
        case 3: return decode_data_24(bmp); break;
        default: return -1;
    }
    return -1;
}

int encode_data_24(bmp_file_t *bmp)
{
    int32_t line    = bmp->image->height;
    int32_t column  = 0;

    float   *src;
    int32_t samp;
    uint8_t *dst;

    int32_t width;

    bmp->width_fixed = width = bmp->image->width * 3;

    if (bmp->width_fixed % 4 != 0) {
        bmp->width_fixed = (bmp->width_fixed - bmp->width_fixed % 4 + 4);
    }

    bmp->data_size = bmp->width_fixed * line * sizeof(uint8_t);

    bmp->raw_data = malloc(bmp->data_size);

    dst = bmp->raw_data;

    for (line--; line >= 0; line--) {
        src = &((float*)bmp->image->data)[line * width];
        for(column = 0; column < width; column++) {
            samp = round(*src);
            if (samp > 255) samp = 255;
            if (samp < 0) samp = 0;
            *dst = samp;
            src ++;
            dst ++;
        }
        while (column < bmp->width_fixed) {
            *dst = 0;
            column ++;
            dst ++;
        }
    }

    return 0;
}

int encode_data(bmp_file_t *bmp)
{
    return encode_data_24(bmp);
}

/**************************************************

    BMP Load && Save

**************************************************/

image_t * bmp_load(const char *file_name)
{
    bmp_file_t      bmp;

    do {
        /* open file */
        bmp.fp = fopen(file_name, "rb");

        if(bmp.fp == NULL){
            fprintf(stderr, "Cannot open file.\n");
            break;
        }

        if (read_header(&bmp) != 0) break;
        if (read_info(&bmp) != 0)   break;
        if (read_data(&bmp) != 0)   break;

        bmp.image = image_new(bmp.info.biWidth, bmp.info.biHeight, IMG_MODEL_BGR);

        if (bmp.image == NULL) break;

        if (decode_data(&bmp) != 0) break;

        free(bmp.raw_data);

        fclose(bmp.fp);

        return bmp.image;

    } while (0);

    if (bmp.raw_data != NULL) free(bmp.raw_data);
    if (bmp.image != NULL) image_free(bmp.image);
    return NULL;
}

int bmp_save(image_t * img, const char *file_name)
{
    bmp_file_t      bmp;

    uint32_t  data_offset;
    uint32_t  file_size;

    do {
        //image_convert(img, IMG_MODEL_BGR);

        memset(&bmp, 0, sizeof(bmp_file_t));

        /* open file */
        bmp.fp = fopen(file_name, "wb");

        if(bmp.fp == NULL){
            fprintf(stderr, "Cannot open file.\n");
            break;
        }

        bmp.image = img;

        if(encode_data(&bmp) != 0) break;

        data_offset = sizeof(bmp_header_t) + sizeof(bmp_info_t);
        file_size   = data_offset + bmp.data_size;

        str2_BE(bmp.header.bfType,    WINDOWS_BMP);
        str4_BE(bmp.header.bfSize,    file_size);
        str4_BE(bmp.header.bfOffBits, data_offset);

        bmp.info.biSize          = DIB_BITMAPINFOHEADER_SIZE;
        bmp.info.biWidth         = bmp.image->width;
        bmp.info.biHeight        = bmp.image->height;
        bmp.info.biPlanes        = 1; /* This shoule always be 1 */
        bmp.info.biBitCount      = 24;
        bmp.info.biCompression   = COMPRESS_BI_RGB;
        bmp.info.biSizeImage     = bmp.data_size;
        bmp.info.biXPelsPerMeter = 72;
        bmp.info.biYPelsPerMeter = 72;
        bmp.info.biClrUsed       = 0;
        bmp.info.biClrImportant  = 0;

        fwrite(&bmp.header, sizeof(bmp_header_t), 1, bmp.fp);
        fwrite(&bmp.info, sizeof(bmp_info_t), 1, bmp.fp);
        fwrite(bmp.raw_data, bmp.data_size, 1, bmp.fp);

        fclose(bmp.fp);

        free(bmp.raw_data);

        return 0;

    } while (0);

    return -1;
}
