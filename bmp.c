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

    uint32_t        data_offset;
    uint32_t        color_depth;
    uint32_t        width_fixed;
    uint32_t        data_size;
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
    uint32_t column  = 0;

    uint8_t *src;
    uint8_t *dst = bmp->image->data;

    uint32_t width;

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
    uint32_t column  = 0;

    uint8_t *src;
    uint8_t *dst;

    uint32_t width;

    bmp->width_fixed = width = bmp->image->width * 3;

    if (bmp->width_fixed % 4 != 0) {
        bmp->width_fixed = (bmp->width_fixed - bmp->width_fixed % 4 + 4);
    }

    bmp->data_size = bmp->width_fixed * line * sizeof(uint8_t);

    bmp->raw_data = malloc(bmp->data_size);

    dst = bmp->raw_data;

    for (line--; line >= 0; line--) {
        src = bmp->image->data + line * width;
        for(column = 0; column < width; column++) {
            *dst = *src;
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

        bmp.image = img_new(bmp.info.biWidth, bmp.info.biHeight, IMG_MODEL_BGR);

        if (bmp.image == NULL) break;

        if (decode_data(&bmp) != 0) break;

        free(bmp.raw_data);

        return bmp.image;

    } while (0);

    if (bmp.raw_data != NULL) free(bmp.raw_data);
    if (bmp.image != NULL) img_free(bmp.image);
    return NULL;
}

int bmp_save(image_t * img, const char *file_name)
{
    bmp_file_t      bmp;

    uint32_t  data_offset;
    uint32_t file_size;

    do {
        img_convert(img, IMG_MODEL_BGR);

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

#if 0

/* save bmp to file by default settings */
/* bmp_format = Windows 3.1x bitmap (424D) */
/* dib_format = Standard BMP header */
/* bit_pre_pixel = 24  */
int tlb_save_bmp(tlb_image_t * image, const char *file_name)
{
    /* bmp header struct */
    bmp_header_t header;

    /* bmp information header */
    bmp_stddib_t dib_info;

    uint8_t  data_offset;
    uint32_t data_size;
    uint32_t file_size;

    uint8_t *bmp_data = NULL;

    FILE * fp = NULL;

    if(image==NULL){
        fprintf(stderr, "%s(): Cannot operation with (null) image.\n",__FUNCTION__);
        return TLB_ERROR;
    }

    /* convert to bmp data */
    bmp_data = convert_to_raw(image, 3, &data_size);

    /* convert exceptional */
    if(bmp_data == NULL){
        fprintf(stderr, "Cannot convert this file.\n");
        return TLB_ERROR;
    }

    data_offset = sizeof(bmp_header_t) + sizeof(bmp_stddib_t);
    file_size   = data_offset + data_size;

    /* bmp header magic */
    memset(&header, 0, sizeof(bmp_header_t));

    str2_LE(header.bfType,    WINDOWS_BMP);
    str4_BE(header.bfSize,    file_size);
    str4_BE(header.bfOffBits, data_offset);

    dib_info.biSize          = DIB_BITMAPINFOHEADER_SIZE;
    dib_info.biWidth         = image->width;
    dib_info.biHeight        = image->height;
    dib_info.biPlanes        = 1; /* This shoule always be 1 */
    dib_info.biBitCount      = 24;
    dib_info.biCompression   = COMPRESS_BI_RGB;
    dib_info.biSizeImage     = data_size;
    dib_info.biXPelsPerMeter = 72;
    dib_info.biYPelsPerMeter = 72;
    dib_info.biClrUsed       = 0;
    dib_info.biClrImportant  = 0;

    /* open file */
    fp = fopen(file_name,"wb");

    /* open exceptional */
    if(fp == NULL){
        fprintf(stderr, "Cannot open file.\n");
        return TLB_ERROR;
    }

    fwrite(&header, sizeof(header), 1, fp);
    fwrite(&dib_info, sizeof(dib_info), 1, fp);
    fwrite(bmp_data, data_size, 1, fp);

    fclose(fp);
    s_free(bmp_data);

    return 0;

}


/**************/
/* conv utils */
/**************/
tlb_core_t * tlb_core_new(uint8_t size)
{
    tlb_core_t * core = NULL;
    int16_t real_size = 0;

    if (size<=1){
        fprintf(stderr, "Error: Convolution core too small.\n");
        return NULL;
    }

    real_size = size * size;

    core = malloc(sizeof(tlb_core_t));

    /* malloc exceptional */
    if(core == NULL){
        fprintf(stderr, "Error: Can not malloc for new object.\n");
        return NULL;
    }

    /* init */
    core->size = size;
    core->div  = 0.0;
    core->data = malloc(real_size*sizeof(double));

    /* malloc exceptional */
    if(core->data == NULL){
        fprintf(stderr, "Error: Can not malloc for new object.\n");
        return NULL;
    }

    for (real_size = real_size - 1; real_size >= 0; real_size--){
        core->data[real_size] = 0.0;
    }

    return core;
}

int __cdecl tlb_core_load(tlb_core_t * core, ...)
{
    va_list  sp;
    uint16_t cnt;
    uint16_t size = core->size*core->size;

    va_start(sp, core);
    for (cnt = 0; cnt < size; cnt++){
        core->data[cnt] = va_arg(sp, double);
    }
    va_end(sp);
    return 0;
}

// double tlb_core_element(tlb_core_t * core, uint8_t x, uint8_t y)
// {
//     uint16_t p;
//     p = core->size * y + x;
//     return core->data[p];
// }

int tlb_core_standard(tlb_core_t * core)
{
    uint16_t real_size;
    uint16_t index;
    double   sum = 0;
    real_size = core->size * core->size;
    for(index = 0; index < real_size; index++){
        sum +=  core->data[index];
    }
    if(sum < -0.000001){
        core->div  = -sum;
        core->bias = 255;
    }else if(sum < 0.000001){
        /* core->div == 0 */
        core->div  = 1.0;
        core->bias = 128;
    }else{
        core->div = sum;
    }

    return TLB_OK;
}

/***************/
/* Image utils */
/***************/
tlb_image_t * tlb_img_new(uint32_t width, uint32_t height, color_t bgcolor)
{
    tlb_image_t * image = NULL;

    uint32_t * data;
    uint32_t index;

    /* get memory for image */
    image = malloc(sizeof(tlb_image_t));

    /* malloc exceptional */
    if(image == NULL){
        fprintf(stderr, "Error: Can not malloc for new object.\n");
        return NULL;
    }

    /* set width of struct */
    image->width  = width;

    /* set height of struct */
    image->height = height;

    /* malloc for cooked img data */
    image->data = malloc((4*image->width*image->height)*sizeof(uint8_t));
    if(image->data == NULL){
        fprintf(stderr, "Error: Can not malloc for new object.\n");
        return NULL;
    }

    data = (uint32_t*)(image->data);

    for(index = 0; index < width*height; index++)
    {
        data[index] = bgcolor;
    }

    return image;
}

void tlb_img_free(tlb_image_t * image)
{
    if(image != NULL){
        s_free(image->data);
        s_free(image);
    }
}

tlb_image_t * tlb_img_copy(tlb_image_t * src_image)
{
    tlb_image_t * image = NULL;

    if(src_image==NULL){
        fprintf(stderr, "%s(): Cannot operation with (null) image.\n",__FUNCTION__);
        return NULL;
    }

    /* get memory for image */
    image = malloc(sizeof(tlb_image_t));

    /* malloc exceptional */
    if(image == NULL){
        fprintf(stderr, "Error: Can not malloc for new object.\n");
        return NULL;
    }

    /* set width of struct */
    image->width  = src_image->width;

    /* set height of struct */
    image->height = src_image->height;

    /* malloc for cooked img data */
    image->data = malloc((4*image->width*image->height)*sizeof(char));
    if(image->data == NULL){
        fprintf(stderr, "Error: Can not malloc for new object.\n");
        return NULL;
    }

    memcpy(image->data, src_image->data, (4*image->width*image->height));

    return image;
}

tlb_image_t * tlb_img_chop(tlb_image_t * image, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)
{
    tlb_image_t * chop = NULL;
    uint32_t x,y;

    if(image==NULL){
        fprintf(stderr, "%s(): Cannot operation with (null) image.\n",__FUNCTION__);
        return NULL;
    }

    if((x0==x1)||(y0==y1)){
        fprintf(stderr, "Error: Selected area too small.\n");
        return NULL;
    }

    /* make point 1 on left-down and point 2 on right-up */
    if(x0 > x1) tlb_swap(x0, x1);
    if(y0 > y1) tlb_swap(y0, y1);

    /* adjust for large area */
    x1 = (x1>image->width)  ? image->width  : x1 ;
    y1 = (y1>image->height) ? image->height : y1 ;

    chop = tlb_img_new(x1-x0, y1-y0, 0);

    for (x = x0; x < x1; x++){
        for (y = y0; y < y1; y++){
            *(uint32_t*)tlb_pixel(chop, x-x0, y-y0) = *(uint32_t*)tlb_pixel(image, x, y);
        }
    }

    return chop;
}

int tlb_img_paste(tlb_image_t * src, tlb_image_t * dst, uint32_t x0, uint32_t y0){
    uint32_t xs,ys;
    uint32_t xd,yd;
    /* check if NULL pointer */
    if(dst==NULL||src==NULL){
        fprintf(stderr, "%s(): Cannot operation with (null) image.\n",__FUNCTION__);
        return TLB_ERROR;
    }

    ys = 0;
    yd = y0;

    while((ys<src->height)&&(yd<dst->height)){

        xs = 0;
        xd = x0;

        while((xs<src->width)&&(xd<dst->width)){
            /* copy by uint32_t */
            *(uint32_t*)tlb_pixel(dst, xd, yd) = *(uint32_t*)tlb_pixel(src, xs, ys);

            xs++;
            xd++;
        }

        ys++;
        yd++;
    }

    return TLB_OK;
}

tlb_image_t * tlb_img_make_border(tlb_image_t * image, uint32_t brsize, uint8_t type){
    tlb_image_t * border = NULL;
    uint32_t x,y;
    /* check if NULL pointer */
    if(image==NULL){
        fprintf(stderr, "%s(): Cannot operation with (null) image.\n",__FUNCTION__);
        return NULL;
    }
    /* if brsize==0 it degenerate to tlb_img_copy */
    if(brsize==0){
        return tlb_img_copy(image);
    }

    border = tlb_img_new(image->width+(2*brsize),image->height+(2*brsize),0);

    tlb_img_paste(image, border, brsize, brsize);

    if(type == TLB_BORDER_EMPTY){
        return border;
    }

    if(type == TLB_BORDER_REPLICATE){

        /* up and down */
        for(x = brsize; x < image->width+brsize; x++){
            for(y = 0; y < brsize; y++){
                *(uint32_t*)tlb_pixel(border, x, y) =\
                *(uint32_t*)tlb_pixel(border, x, brsize);
                *(uint32_t*)tlb_pixel(border, x, border->height - y - 1) =\
                *(uint32_t*)tlb_pixel(border, x, border->height - brsize - 1);
            }
        }

        /* left and right */
        for(y = brsize; y < image->height+brsize; y++){
            for(x = 0; x < brsize; x++){
                *(uint32_t*)tlb_pixel(border, x, y) =\
                *(uint32_t*)tlb_pixel(border, brsize, y);
                *(uint32_t*)tlb_pixel(border, border->width - x - 1, y) =\
                *(uint32_t*)tlb_pixel(border, border->width - brsize - 1, y);
            }
        }

        /* corner */
        for(y = 0; y < brsize; y++){
            for(x = 0; x < brsize; x++){
                *(uint32_t*)tlb_pixel(border, x, y) =\
                *(uint32_t*)tlb_pixel(border, brsize, brsize);
                *(uint32_t*)tlb_pixel(border, border->width-x-1, y) =\
                *(uint32_t*)tlb_pixel(border, border->width-brsize-1, brsize);
                *(uint32_t*)tlb_pixel(border, x, border->height-y-1) =\
                *(uint32_t*)tlb_pixel(border, brsize, border->height-brsize-1);
                *(uint32_t*)tlb_pixel(border, border->width-x-1, border->height-y-1) =\
                *(uint32_t*)tlb_pixel(border, border->width-brsize-1, border->height-brsize-1);
            }
        }
        return border;
    }

    fprintf(stderr, "%s(): need to specific a border type, treated as TLB_BORDER_EMPTY.\n",__FUNCTION__);
    return border;
}

tlb_image_t * tlb_img_conv_r(tlb_image_t * image, tlb_core_t * core, uint8_t channel){
    tlb_image_t * conv = NULL;
    uint32_t x0;
    uint32_t y0;
    uint32_t x1;
    uint32_t y1;
    uint32_t d;
    double   tmp[4];
    if(image==NULL||core==NULL){
        fprintf(stderr, "%s(): Cannot operation with (null) image.\n",__FUNCTION__);
        return NULL;
    }
    d = (core->size / 2);

    conv = tlb_img_copy(image);

    if(channel!=CHANNEL_ALL){
        for(x0 = 0; x0<image->width-core->size; x0++){
            for(y0 = 0; y0<image->height-core->size; y0++){
                /* ergodic pixel in image */
                tmp[0] = 0;
                for(x1 = 0; x1<core->size; x1++){
                    for(y1 = 0; y1<core->size; y1++){
                        tmp[0] += tlb_core_element(core, x1, y1) * (tlb_pixel(image, x0+x1, y0+y1)[channel]);
                    }
                }
                tmp[0] = tmp[0]/core->div + core->bias;

                if (tmp[0]>255) tmp[0] = 255;
                if (tmp[0]<0)   tmp[0] = 0;
                tlb_pixel(conv, x0+d, y0+d)[channel] = tmp[0];
                
            }
        }
    }else{
        for(x0 = 0; x0<image->width-core->size; x0++){
            for(y0 = 0; y0<image->height-core->size; y0++){
                /* ergodic pixel in image */
                tmp[CHANNEL_R] = 0;
                tmp[CHANNEL_G] = 0;
                tmp[CHANNEL_B] = 0;
                tmp[CHANNEL_A] = 0;

                for(x1 = 0; x1<core->size; x1++){
                    for(y1 = 0; y1<core->size; y1++){
                        tmp[CHANNEL_R] += tlb_core_element(core, x1, y1) * (tlb_pixel(image, x0+x1, y0+y1)[CHANNEL_R]);
                        tmp[CHANNEL_G] += tlb_core_element(core, x1, y1) * (tlb_pixel(image, x0+x1, y0+y1)[CHANNEL_G]);
                        tmp[CHANNEL_B] += tlb_core_element(core, x1, y1) * (tlb_pixel(image, x0+x1, y0+y1)[CHANNEL_B]);
                        tmp[CHANNEL_A] += tlb_core_element(core, x1, y1) * (tlb_pixel(image, x0+x1, y0+y1)[CHANNEL_A]);
                    }
                }

                tmp[CHANNEL_R] = tmp[CHANNEL_R]/core->div + core->bias;
                tmp[CHANNEL_G] = tmp[CHANNEL_G]/core->div + core->bias;
                tmp[CHANNEL_B] = tmp[CHANNEL_B]/core->div + core->bias;
                tmp[CHANNEL_A] = tmp[CHANNEL_A]/core->div + core->bias;

                if (tmp[CHANNEL_R]>255) tmp[CHANNEL_R] = 255;
                if (tmp[CHANNEL_G]>255) tmp[CHANNEL_G] = 255;
                if (tmp[CHANNEL_B]>255) tmp[CHANNEL_B] = 255;
                if (tmp[CHANNEL_A]>255) tmp[CHANNEL_A] = 255;

                if (tmp[CHANNEL_R]<0)   tmp[CHANNEL_R] = 0;
                if (tmp[CHANNEL_G]<0)   tmp[CHANNEL_G] = 0;
                if (tmp[CHANNEL_B]<0)   tmp[CHANNEL_B] = 0;
                if (tmp[CHANNEL_A]<0)   tmp[CHANNEL_A] = 0;

                tlb_pixel(conv, x0+d, y0+d)[CHANNEL_R] = tmp[CHANNEL_R];
                tlb_pixel(conv, x0+d, y0+d)[CHANNEL_G] = tmp[CHANNEL_G];
                tlb_pixel(conv, x0+d, y0+d)[CHANNEL_B] = tmp[CHANNEL_B];
                tlb_pixel(conv, x0+d, y0+d)[CHANNEL_A] = tmp[CHANNEL_A];
                
            }
        }
    }
    return conv;
}

tlb_image_t * tlb_img_conv(tlb_image_t * image, tlb_core_t * core, uint8_t channel){
    tlb_image_t * border = NULL;
    tlb_image_t * r_conv = NULL;
    tlb_image_t * c_conv = NULL;

    if(image==NULL){
        fprintf(stderr, "%s(): Cannot operation with (null) image.\n",__FUNCTION__);
        return NULL;
    }

    if(core==NULL){
        fprintf(stderr, "%s(): Cannot operation with (null) conv core.\n",__FUNCTION__);
        return NULL;
    }

    if(core->div==0){
        fprintf(stderr, "%s(): Illegal conv core.\n",__FUNCTION__);
        return NULL;
    }

    border = tlb_img_make_border(image, core->size, TLB_BORDER_REPLICATE);

    if(border==NULL){
        fprintf(stderr, "%s(): Cannot do copy and make border operation.\n",__FUNCTION__);
        return NULL;
    }

    r_conv = tlb_img_conv_r(border, core, channel);

    tlb_img_free(border);

    if(r_conv==NULL){
        fprintf(stderr, "%s(): Cannot do convolution operation.\n",__FUNCTION__);
        return NULL;
    }

    c_conv = tlb_img_chop(r_conv, core->size, core->size, image->width + core->size, image->height + core->size);

    tlb_img_free(r_conv);

    if(c_conv==NULL){
        fprintf(stderr, "%s(): Cannot do chop operation.\n",__FUNCTION__);
        return NULL;
    }

    return c_conv;
}

/*******************/
/* pixel operation */
/*******************/
/* get a array of the pixel */
uint8_t * tlb_pixel(tlb_image_t * image, uint32_t x, uint32_t y){
    /* Check the input legitimacy */
    if(x > image->width){
        return NULL;
    }
    if(y > image->height){
        return NULL;
    }
    return (image->data + 4*(y * image->width + x));
}

/* print a pixel with specific color */
int tlb_pixel_set(tlb_image_t * image, uint32_t x, uint32_t y, color_t color){
    if(x > image->width){
        return TLB_ERROR;
    }
    if(y > image->height){
        return TLB_ERROR;
    }
    *(uint32_t*)(image->data + 4*(y * image->width + x)) = color;
    return TLB_OK;
}

/* print a pixel with specific color */
int tlb_pixel_ch_set(tlb_image_t * image, uint32_t x, uint32_t y, uint8_t channel, uint8_t val){
    if(x > image->width){
        return TLB_ERROR;
    }
    if(y > image->height){
        return TLB_ERROR;
    }
    (image->data + 4*(y * image->width + x))[channel] = val;
    return TLB_OK;
}

/*******************/
/* block operation */
/*******************/

/* get the average color of a block */
uint32_t tlb_block_average(tlb_image_t * image,\
                        uint32_t offset_x, uint32_t offset_y,\
                        uint32_t length_x, uint32_t length_y)
{
    uint32_t color_sum[4] = {0};
    uint8_t  color_ave[4] = {0};

    uint32_t pixel_x = 0;
    uint32_t pixel_y = 0;

    uint8_t* pixel = NULL;

    uint32_t block_size = length_x * length_y;

    /* select a pixel in block */
    for (pixel_x = 0; pixel_x < length_x; pixel_x++){
        for (pixel_y = 0; pixel_y < length_y; pixel_y++){

            /* get the pointer of the pixel */
            pixel = tlb_pixel(image, offset_x + pixel_x, offset_y + pixel_y);
            if (pixel != NULL){
                color_sum[CHANNEL_R] += pixel[CHANNEL_R];
                color_sum[CHANNEL_G] += pixel[CHANNEL_G];
                color_sum[CHANNEL_B] += pixel[CHANNEL_B];
                color_sum[CHANNEL_A] += pixel[CHANNEL_A];
            }

        }
    }

    color_ave[CHANNEL_R] = color_sum[CHANNEL_R] / block_size;
    color_ave[CHANNEL_G] = color_sum[CHANNEL_G] / block_size;
    color_ave[CHANNEL_B] = color_sum[CHANNEL_B] / block_size;
    color_ave[CHANNEL_A] = color_sum[CHANNEL_A] / block_size;

    return tlb_rgba(color_ave[CHANNEL_R],\
                    color_ave[CHANNEL_G],\
                    color_ave[CHANNEL_B],\
                    color_ave[CHANNEL_A]);
}

/* fill a block with a specific color */
int tlb_block_fill(tlb_image_t * image,\
                uint32_t offset_x, uint32_t offset_y,\
                uint32_t length_x, uint32_t length_y,\
                color_t color)
{
    uint32_t pixel_x = 0;
    uint32_t pixel_y = 0;

    uint32_t* pixel = NULL;

    /* select a pixel in block */
    for (pixel_x = 0; pixel_x < length_x; pixel_x++){
        for (pixel_y = 0; pixel_y < length_y; pixel_y++){

            /* get the pointer of the pixel */
            pixel = (uint32_t*)tlb_pixel(image, offset_x + pixel_x, offset_y + pixel_y);
            if (pixel != NULL){
                *pixel = color;
            }

        }
    }

    return TLB_OK;
}

tlb_image_t * tlb_block_mosaic(tlb_image_t * image,\
                            uint32_t offset_x, uint32_t offset_y,\
                            uint32_t length_x, uint32_t length_y,\
                            uint32_t granularity)
{
    tlb_image_t * mosaic = NULL;

    uint32_t color   = 0;

    uint32_t block_x = 0;
    uint32_t block_y = 0;

    mosaic = tlb_img_copy(image);

    for (block_x = granularity; block_x < length_x; block_x+=granularity){
        for (block_y = granularity; block_y < length_y; block_y+=granularity){

            color = tlb_block_average(mosaic,\
                block_x + offset_x,\
                block_y + offset_y,\
                granularity,\
                granularity);

            tlb_block_fill(mosaic,\
                block_x + offset_x,\
                block_y + offset_y,\
                granularity,\
                granularity,\
                color);
        }
    }

    return mosaic;
}

/********************/
/* drowing in image */
/********************/

int tlb_draw_line(tlb_image_t * image,\
    uint32_t x0, uint32_t y0, \
    uint32_t x1, uint32_t y1, \
    color_t color)
{
    /* I have use a lot of inline sentences in this function */
    /* it was not for speed, just bucause I'm lazy */
    uint8_t  steep = 0;
    uint32_t x;
    uint32_t y;
    uint32_t dx;
    uint32_t dy;
    float    t;

    /* inline abs(x0-x1) */
    if(x0 > x1){
        dx = x0 - x1;
    }else{
        dx = x1 - x0;
    }

    /* inline abs(y0-y1) */
    if(y0 > y1){
        dy = y0 - y1;
    }else{
        dy = y1 - y0;
    }


    if (dx<dy&&dx!=0&&dy!=0&&dx!=dy){
        /* if the line is steep, rotate the canvas */
        tlb_swap(x0, y0);
        tlb_swap(x1, y1);
        steep = 1;
    }

    if (x0>x1) {
        /* make line from left to right */
        tlb_swap(x0, x1);
        tlb_swap(y0, y1);
    }

    if(dx!=0&&dy!=0&&dx!=dy){
        /* Draw line with Bresenham’s Line Drawing Algorithm */
        for(x = x0; x <= x1; x++){
            t = (x-x0) / (float)(x1-x0);
            y = y0 * (1.0-t) + y1 * t;
            if (steep) { 
                tlb_pixel_set(image, y, x, color);
            } else { 
                tlb_pixel_set(image, x, y, color); 
            } 
        }
    }else if(dx==dy&&y0<y1){
        /* k = 1 line */
        for(x = x0, y = y0; x <= x1; x++, y++){
                tlb_pixel_set(image, x, y, color); 
        }
    }else if(dx==dy&&y0>y1){
        /* k = -1 line */
        for(x = x0, y = y0; x <= x1; x++, y--){
                tlb_pixel_set(image, x, y, color); 
        }
    }else if(dx==0){
        /* k = 0 line */
        x = x0;
        y = y0<y1?y0:y1;
        y1 = y0>y1?y0:y1;
        y0 = y;
        for(y = y0; y <= y1; y++){
                tlb_pixel_set(image, x, y, color); 
        }
    }else if(dy==0){
        /* k = ∞ line */
        for(x = x0, y = y0; x <= x1; x++){
                tlb_pixel_set(image, x, y, color); 
        }
    }else{
        return TLB_ERROR;
    }

    return TLB_OK;
}

int tlb_draw_triangle(\
    tlb_image_t * image,\
    uint32_t x0,\
    uint32_t y0,\
    uint32_t x1,\
    uint32_t y1,\
    uint32_t x2,\
    uint32_t y2,\
    color_t color)
{
    int32_t height;
    int32_t i,j;
    float   k_a;
    float   k_b;
    int32_t x_a;
    int32_t x_b;
    int32_t seg_height;
    volatile int8_t  half_height = 0;

    if (y0==y1 && y0==y2){
        return TLB_ERROR;   
    }

    /* sort the point by y */
    if(y0>y1){
        tlb_swap(x0, x1);
        tlb_swap(y0, y1);
    }
    if(y0>y2){
        tlb_swap(x0, x2);
        tlb_swap(y0, y2);
    }
    if(y1>y2){
        tlb_swap(x1, x2);
        tlb_swap(y1, y2);
    }

    height = y2-y0;
    for (i = 0; i < height; i++){
        if(y1==y0){
            half_height = 1;
        }else if(i>=(int32_t)(y1-y0)){
            half_height = 1;
        }

        seg_height  = half_height ? y2-y1 : y1-y0;

        k_a = (float)i/height;
        k_b = (float)(i - (half_height ? y1-y0 : 0))/seg_height;

        x_a = x0 + k_a * (int32_t)(x2 - x0);
        x_b = half_height ? x1 + k_b * (int32_t)(x2 - x1) : x0 + k_b * (int32_t)(x1 - x0);
        if(x_a>x_b){
            tlb_swap(x_a,x_b);
        }
        for(j = x_a; j < x_b; j++){
            tlb_pixel_set(image, j, i+y0, color);
        }
    }

    // tlb_pixel_set(image, x0, y0, tlb_rgb(255,255,255));
    // tlb_pixel_set(image, x1, y1, tlb_rgb(255,255,255));
    // tlb_pixel_set(image, x2, y2, tlb_rgb(255,255,255));

    return TLB_OK;
}


/*******************/
/* image operation */
/*******************/

int tlb_img_inverse(tlb_image_t * image)
{
    uint32_t size;
    uint8_t  *now;
    uint8_t  *last;

    size = image->width * image->height * CHANNEL_CNT;

    last = image->data + size - CHANNEL_CNT;

    for(now = image->data; now<=last; now+=CHANNEL_CNT){
        now[CHANNEL_R] = 0xFF - now[CHANNEL_R];
        now[CHANNEL_G] = 0xFF - now[CHANNEL_G];
        now[CHANNEL_B] = 0xFF - now[CHANNEL_B];
    }
    return 0;
}

int tlb_img_gray(tlb_image_t * image)
{
    uint32_t size;
    uint8_t  *now;
    uint8_t  *last;

    uint8_t gray;

    size = image->width * image->height * CHANNEL_CNT;

    last = image->data + size - CHANNEL_CNT;

    for(now = image->data; now<=last; now+=CHANNEL_CNT){
        gray = (now[CHANNEL_R] + now[CHANNEL_G] + now[CHANNEL_B])/3;
        now[CHANNEL_R] = gray;
        now[CHANNEL_G] = gray;
        now[CHANNEL_B] = gray;
    }
    return 0;
}

int tlb_img_binary(tlb_image_t * image, uint8_t threshold){
    uint32_t size;
    uint8_t  *now;
    uint8_t  *last;

    uint8_t gray;

    size = image->width * image->height * CHANNEL_CNT;

    last = image->data + size - CHANNEL_CNT;

    for(now = image->data; now<=last; now+=CHANNEL_CNT){
        gray = (now[CHANNEL_R] + now[CHANNEL_G] + now[CHANNEL_B])/3;
        if(gray > threshold){
            *(uint32_t*)now |= 0x00FFFFFF;
        }else{
            *(uint32_t*)now &= 0xFF000000;
        }
    }
    return 0;
}

tlb_image_t * tlb_img_channel(tlb_image_t * image, uint8_t channel)
{
    tlb_image_t * ext = NULL;

    uint32_t pixel_index;
    uint32_t pixel_cnt;
    uint32_t pixel_mask;
    uint32_t *img_data;

    ext = tlb_img_copy(image);

    pixel_mask = 0x000000FF << 8*channel;
    pixel_cnt  = ext->width * ext->height;
    img_data   = (uint32_t*)ext->data;

    for (pixel_index = 0; pixel_index<pixel_cnt; pixel_index++){
        img_data[pixel_index] = img_data[pixel_index]&pixel_mask;
    }

    return ext;
}

int tlb_img_color_replace(tlb_image_t * image, color_t find, color_t replace)
{
    uint32_t size = image->width * image->height;
    uint32_t *data = (uint32_t*)image->data;
    uint32_t i;

    for (i = 0; i < size; ++i){
        if(data[i]==find) data[i]=replace;
    }
    return 0;
}

int tlb_img_color_switch(tlb_image_t * image, color_t color1, color_t color2)
{
    uint32_t size = image->width * image->height;
    uint32_t *data = (uint32_t*)image->data;
    uint32_t i;

    for (i = 0; i < size; ++i){
        if(data[i]==color1){
            data[i]=color2;
        }else if(data[i]==color2){
            data[i]=color1;
        }
    }
    return 0;
}

tlb_image_t * tlb_img_ch_histogram(tlb_image_t * image, uint8_t channel)
{
    tlb_image_t * histogram = NULL;

    /* ver summary */
    uint32_t summary[256] = {0};

    /* image_size */
    uint32_t size = image->width * image->height;

    /* element index */
    long int i,j;

    uint32_t cnt_max = 0;

    for (i = channel; i < 4 * size; i += CHANNEL_CNT){
        summary[image->data[i]]++;
    }

    for (i = 0; i < 256; i++){
        cnt_max = (summary[i] > cnt_max) ? summary[i] : cnt_max;
    }

    for (i = 0; i < 256; i++){
        summary[i] = (summary[i]*100)/cnt_max;
    }

    histogram = tlb_img_new(256, 100, tlb_rgba(0,0,0,0));

    for(i = 0; i < 256; i++){
        for(j = summary[i]; j>=0; j--){
            tlb_pixel_ch_set(histogram, i, j, channel, 0xFF);
        }
    }
    return histogram;
}

tlb_image_t * tlb_img_histogram(tlb_image_t * image)
{
    tlb_image_t * histogram = NULL;

    uint8_t channel = 0;

    /* image_size */
    uint32_t size = image->width * image->height;

    histogram = tlb_img_new(512, 200, tlb_rgba(255,255,255,0));

    for (channel = 0; channel < CHANNEL_CNT; ++channel){
        /* ver summary */
        uint32_t summary[256] = {0};

        /* element index */
        long int i,j;

        uint32_t cnt_max = 0;

        for (i = channel; i < 4 * size; i += CHANNEL_CNT){
            summary[image->data[i]]++;
        }

        for (i = 0; i < 255; i++){
            cnt_max = (summary[i] > cnt_max) ? summary[i] : cnt_max;
        }

        for (i = 0; i < 256; i++){
            summary[i] = (summary[i]*200)/cnt_max;
            if(summary[i]>199) summary[i]=199;
        }

        for(i = 0; i < 256; i++){
            for(j = summary[i]; j>=0; j--){
                tlb_pixel_ch_set(histogram, (2*i), j, channel, 0x00);
            }
            for(j = summary[i]; j>=0; j--){
                tlb_pixel_ch_set(histogram, (2*i)+1, j, channel, 0x00);
            }
        }
    }

    tlb_img_inverse(histogram);
    tlb_img_color_replace(histogram, tlb_rgba(255,0,0,0), tlb_rgba(255,160,160,0));
    tlb_img_color_replace(histogram, tlb_rgba(0,255,0,0), tlb_rgba(160,255,160,0));
    tlb_img_color_replace(histogram, tlb_rgba(0,0,255,0), tlb_rgba(160,160,255,0));

    tlb_img_color_replace(histogram, tlb_rgba(255,255,0,0), tlb_rgba(210,255,150,0));
    tlb_img_color_replace(histogram, tlb_rgba(255,0,255,0), tlb_rgba(210,150,255,0));
    tlb_img_color_replace(histogram, tlb_rgba(0,255,255,0), tlb_rgba(180,210,255,0));

    tlb_img_color_replace(histogram, tlb_rgba(0,0,0,0), tlb_rgba(230,230,230,0));
    tlb_img_color_replace(histogram, tlb_rgba(255,255,255,0), tlb_rgba(100,150,200,0));

    return histogram;
}

tlb_image_t * tlb_img_mosaic(tlb_image_t * image, uint32_t granularity)
{
    return tlb_block_mosaic(image, 0, 0, image->width, image->height, granularity);
}

/*********************/
/* Private Functions */
/*********************/



int convert_from_raw(tlb_image_t * image, uint32_t depth)
{
    uint8_t * head = image->data;
    uint8_t * src;
    uint8_t * dst;
    uint8_t pixel[4] = {0};

    uint32_t convert_cnt = 0;
    uint32_t width_fixed = 0;
    uint8_t align_bit = 0;

    uint32_t pixel_cnt = 0;

    width_fixed = depth * image->width;
    if(width_fixed%4!=0){
        width_fixed = (width_fixed - width_fixed % 4 + 4);
        align_bit = 4 - (depth * image->width) % 4;
    }

    src = image->data + (width_fixed * image->height - align_bit - depth);
    dst = image->data + (image->width * image->height * CHANNEL_CNT - CHANNEL_CNT);

    if(depth == 3){
        /* 24-bit RGB color mode */
        while(src>head){
            /* Mapping matrix */
            dst[CHANNEL_R] = src[2];
            dst[CHANNEL_G] = src[1];
            dst[CHANNEL_B] = src[0];
            dst[CHANNEL_A] = 0;

            src -= depth;
            dst -= CHANNEL_CNT;
            convert_cnt ++;
            pixel_cnt++;
            if(convert_cnt==image->width){
                for(convert_cnt=0;convert_cnt<align_bit;convert_cnt++){
                    src --;
                }
                convert_cnt = 0;
            }
        }
        /* deal with the last bit */
        pixel[CHANNEL_R] = src[2];
        pixel[CHANNEL_G] = src[1];
        pixel[CHANNEL_B] = src[0];
        pixel[CHANNEL_A] = 0;
        dst[CHANNEL_R] = pixel[CHANNEL_R];
        dst[CHANNEL_G] = pixel[CHANNEL_G];
        dst[CHANNEL_B] = pixel[CHANNEL_B];
        dst[CHANNEL_A] = pixel[CHANNEL_A];
        pixel_cnt++;
        fprintf(stderr, "%d pixel converted\n", pixel_cnt);
    }else if(depth == 4){
        /* 32-bit RGBA color mode */
        do{
            /* Mapping matrix */
            pixel[CHANNEL_R] = src[3];
            pixel[CHANNEL_G] = src[2];
            pixel[CHANNEL_B] = src[1];
            pixel[CHANNEL_A] = src[0];

            dst[CHANNEL_R] = pixel[CHANNEL_R];
            dst[CHANNEL_G] = pixel[CHANNEL_G];
            dst[CHANNEL_B] = pixel[CHANNEL_B];
            dst[CHANNEL_A] = pixel[CHANNEL_A];

            src -= depth;
            dst -= CHANNEL_CNT;
        }while(src>=head);

    }
    return TLB_OK;
}

uint8_t * convert_to_raw(tlb_image_t * image, uint32_t depth, uint32_t * size)
{
    /* converted data */
    uint8_t * data = NULL;

    uint8_t * src;
    uint8_t * dst;

    uint32_t width_fixed = 0;
    uint32_t width_writed = 0;
    uint32_t height_writed = 0;

    /* width need to aligned to 4 byte */
    width_fixed = depth * image->width;
    if(width_fixed%4!=0)
        width_fixed = (width_fixed - width_fixed % 4 + 4);

    /* malloc memory for converted data */
    *size = width_fixed * image->height;
    data = malloc( *size );

    /* malloc exceptional */
    if(data == NULL){
        fprintf(stderr, "Error: Can not malloc for new object.\n");
        return NULL;
    }

    dst = data;
    src = image->data;

    if(depth == 3){
        /* 24-bit RGB color mode */
        fprintf(stderr, "Warning: All data in Alpha area will LOST.\n");
        for(height_writed = 0; height_writed < image->height; height_writed++){
            for(width_writed = 0; width_writed < 3*image->width; width_writed+=3){
                dst[CHANNEL_R] = src[2];
                dst[CHANNEL_G] = src[1];
                dst[CHANNEL_B] = src[0];
                src += CHANNEL_CNT;
                dst += depth;
            }
            while(width_writed < width_fixed){
                *dst = 0x00;
                dst++;
                width_writed++;
            }
        }

    }
    return data;
}

#endif
