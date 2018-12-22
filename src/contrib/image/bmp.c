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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "image.h"


/**************************************************

    BMP Basic Structs

**************************************************/

/* BMP Header Struct */
typedef struct {
    uint16_t bfType;
    uint32_t bfSize;
    uint16_t bfReserved1;
    uint16_t bfReserved2;
    uint32_t bfOffBits;
}__attribute__((packed)) bmp_header_t;

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


/**************************************************

    BMP Strings

**************************************************/

static const char * string_unknown           = "Unknown";
static const char * string_windows_bmp       = \
 "Windows 3.1x bitmap (424D)";
static const char * string_os2_bitmap_array  = \
 "OS/2 struct bitmap array (4241)";
static const char * string_os2_color_icon    = \
 "OS/2 struct color icon (4349)";
static const char * string_os2_color_pointer = \
 "OS/2 const color pointer (4350)";
static const char * string_os2_icon          = \
 "OS/2 struct icon (4943)";
static const char * string_os2_pointer       = \
 "OS/2 pointer (5054)";


static const char * string_bitmapcoreheader   = \
 "BMP core header";
static const char * string_os22xbitmapheader  = \
 "OS/2 BMP header";
static const char * string_bitmapinfoheader   = \
 "Standard BMP header";
static const char * string_bitmapv2infoheader = \
 "Adobe Photoshop Externed BMP header ver.2";
static const char * string_bitmapv3infoheader = \
 "Adobe Photoshop Externed BMP header ver.3";
static const char * string_os22xbitmapheader2 = \
 "OS/2 BMP header ver.2";
static const char * string_bitmapv4header     = \
 "Standard BMP header ver.4";
static const char * string_bitmapv5header     = \
 "Standard BMP header ver.5";


static const char * string_compress_bi_rgb             = \
 "None";
static const char * string_compress_bi_rle8            = \
 "RLE 8-bit/pixel";
static const char * string_compress_bi_rle4            = \
 "RLE 4-bit/pixel";
static const char * string_compress_bi_bitfields       = \
 "RGBA (Perhaps Huffman 1D)";
static const char * string_compress_bi_jpeg            = \
 "JPEG image for printing";
static const char * string_compress_bi_png             = \
 "PNG image for printing";
static const char * string_compress_bi_alphabitfields  = \
 "RGBA bit field masks on Windows CE 5.0 with .NET 4.0 or later";
static const char * string_compress_bi_cmyk            = \
 "Windows Metafile CMYK";
static const char * string_compress_bi_cmykrle8        = \
 "Windows Metafile CMYK with RLE 8-bit/pixel";
static const char * string_compress_bi_cmykrle4        = \
 "Windows Metafile CMYK with RLE 4-bit/pixel";


const char * get_bmp_type_string(const int16_t type_word)
{
    switch(type_word)
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
        case DIB_BITMAPCOREHEADER_SIZE   :
            return string_bitmapcoreheader   ; break;
        
        case DIB_OS22XBITMAPHEADER_SIZE  :
            return string_os22xbitmapheader  ; break;
        
        case DIB_BITMAPINFOHEADER_SIZE   :
            return string_bitmapinfoheader   ; break;
        
        case DIB_BITMAPV2INFOHEADER_SIZE :
            return string_bitmapv2infoheader ; break;
        
        case DIB_BITMAPV3INFOHEADER_SIZE :
            return string_bitmapv3infoheader ; break;
        
        case DIB_OS22XBITMAPHEADER2_SIZE :
            return string_os22xbitmapheader2 ; break;
        
        case DIB_BITMAPV4HEADER_SIZE     :
            return string_bitmapv4header     ; break;
        
        case DIB_BITMAPV5HEADER_SIZE     :
            return string_bitmapv5header     ; break;
        
        default:
            return string_unknown;
    }

    return string_unknown;
}

const char * get_bmp_comp_string(uint32_t method){
    switch(method)
    {
        case COMPRESS_BI_RGB            :
            return string_compress_bi_rgb            ; break;
        
        case COMPRESS_BI_RLE8           :
            return string_compress_bi_rle8           ; break;
        
        case COMPRESS_BI_RLE4           :
            return string_compress_bi_rle4           ; break;
        
        case COMPRESS_BI_BITFIELDS      :
            return string_compress_bi_bitfields      ; break;
        
        case COMPRESS_BI_JPEG           :
            return string_compress_bi_jpeg           ; break;
        
        case COMPRESS_BI_PNG            :
            return string_compress_bi_png            ; break;
        
        case COMPRESS_BI_ALPHABITFIELDS :
            return string_compress_bi_alphabitfields ; break;
        
        case COMPRESS_BI_CMYK           :
            return string_compress_bi_cmyk           ; break;
        
        case COMPRESS_BI_CMYKRLE8       :
            return string_compress_bi_cmykrle8       ; break;
        
        case COMPRESS_BI_CMYKRLE4       :
            return string_compress_bi_cmykrle4       ; break;
        
        default:
            return string_unknown;
    }
    return string_unknown;
}

char * bmp_bgr24_pack(image_t * img)
{
    bmp_header_t * header;
    bmp_info_t   * info;

    size_t line_width;
    size_t fixed_width = 0;
    size_t file_size;

    int x,y;

    char * file_buffer = NULL;
    char * file_buf_p  = NULL;

    line_width = img->width * 3;

    if (line_width % 4 != 0) {
        fixed_width = 4 - line_width % 4;
    }
    line_width += fixed_width;

    file_size = line_width * img->height + \
                sizeof(bmp_header_t) + sizeof(bmp_info_t);

    file_buffer = malloc(file_size * sizeof(char));
    if (file_buffer == NULL) {
        perror("Cannot alloc memory for output file.");
        return NULL;
    }

    header = (bmp_header_t*)file_buffer;
    header->bfType        = WINDOWS_BMP;
    header->bfSize        = file_size;
    header->bfOffBits     = sizeof(bmp_header_t) + sizeof(bmp_info_t);

    info = (bmp_info_t*)(file_buffer + sizeof(bmp_header_t));
    info->biSize          = DIB_BITMAPINFOHEADER_SIZE;
    info->biWidth         = img->width;
    info->biHeight        = img->height;
    info->biPlanes        = 1; /* This shoule always be 1 */
    info->biBitCount      = 24;
    info->biCompression   = COMPRESS_BI_RGB;
    info->biSizeImage     = file_size - header->bfOffBits;
    info->biXPelsPerMeter = 72;
    info->biYPelsPerMeter = 72;
    info->biClrUsed       = 0;
    info->biClrImportant  = 0;

    file_buf_p = file_buffer + header->bfOffBits;

    for (y = img->height - 1; y >= 0; y--) {
        for (x = 0; x < img->width; x++) {
            *file_buf_p = image_pixel(img, x, y)[IMG_CHANNEL_B];
            file_buf_p ++;
            *file_buf_p = image_pixel(img, x, y)[IMG_CHANNEL_G];
            file_buf_p ++;
            *file_buf_p = image_pixel(img, x, y)[IMG_CHANNEL_R];
            file_buf_p ++;
        }
        file_buf_p += fixed_width;
    }
    return file_buffer;
}

image_t * bmp_rgb24_parse(char * bmp_file, size_t fsize)
{
    bmp_header_t * header;
    bmp_info_t   * info;
    image_t      * img = NULL;
    char         * file_p;

    size_t line_width;
    size_t fixed_width = 0;

    size_t data_length;

    int x,y;

    header = (bmp_header_t*)bmp_file;
    info = (bmp_info_t*)(bmp_file + sizeof(bmp_header_t));

    line_width = info->biWidth * 3;

    if (line_width % 4 != 0) {
        fixed_width = 4 - line_width % 4;
    }
    line_width += fixed_width;

    data_length = line_width * info->biHeight + \
                    sizeof(bmp_header_t) + sizeof(bmp_info_t);

    if (fsize < data_length) {
        perror("Broken file.");
        return NULL;
    }

    img = image_new(info->biWidth, info->biHeight, IMG_MODEL_BGR);
    if (img == NULL) {
        return NULL;
    }

    file_p = bmp_file + header->bfOffBits;

    for (y = img->height - 1; y >= 0; y--) {
        for (x = 0; x < img->width; x++) {
            image_pixel(img, x, y)[IMG_CHANNEL_B] = *file_p;
            file_p ++;
            image_pixel(img, x, y)[IMG_CHANNEL_G] = *file_p;
            file_p ++;
            image_pixel(img, x, y)[IMG_CHANNEL_R] = *file_p;
            file_p ++;
        }
        file_p += fixed_width;
    }
    return  img;
}

image_t * bmp_parse(char * bmp_file, size_t fsize)
{
    bmp_header_t * header;
    bmp_info_t   * info;

    image_t      * img = NULL;

    header = (bmp_header_t*)bmp_file;
    if (header->bfSize > fsize) {
        perror("Broken file.");
        return NULL;
    }

    info = (bmp_info_t*)(bmp_file + sizeof(bmp_header_t));

    if (info->biBitCount == 24) {
        img = bmp_rgb24_parse(bmp_file, fsize);
    } else if (info->biBitCount == 32) {
        img = image_new(info->biWidth, info->biHeight, IMG_MODEL_BGRA);
    } else {
        perror("Cannot Parse This file.");
        return NULL;
    }

    return img;
}

char* bmp_pack(image_t * image, size_t *fsize)
{
    char * fb;
    bmp_header_t * header;

    /***************************/
    /* Todo Add RGBA32 support */
    /***************************/

    fb = bmp_bgr24_pack(image);
    if (fb == NULL) {
        return NULL;
    }
    header = (bmp_header_t*)fb;
    *fsize = header->bfSize;
    return fb;
}
