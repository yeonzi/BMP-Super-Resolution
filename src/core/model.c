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

#include <core/model.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>

extern isr_model_t isr_model_vgg7yuv;

isr_model_t * isr_models[] = {
	&isr_model_vgg7yuv,
	NULL,
};

#define MODEL_FILE_MAGIC 0x52534959

char * model_read(const char *path)
{
	struct stat statbuf;
    size_t file_size;
    FILE * file_p = NULL;
    char * file_buffer = NULL;
    int    block_cnt;
    uint32_t magic;
    size_t  size;

    stat(path, &statbuf);
    file_size = statbuf.st_size;
    if (file_size <= 8) {
        fprintf(stderr, "Bad model file: File size too small\n");
        free(file_buffer);
        return NULL;
    }

    file_p = fopen(path, "rb");
    if (file_p == NULL) {
        perror("Cannot open model file");
        return NULL;
    }

    file_buffer = malloc(file_size * sizeof(char));
    if (file_buffer == NULL) {
        perror("Cannot alloc memory for model");
        fclose(file_p);
        return NULL;
    }

    block_cnt = fread(file_buffer, 1, file_size, file_p);
    fclose(file_p);
    if (block_cnt == 0) {
        perror("Cannot read model file");
        free(file_buffer);
        return NULL;
    }

    magic = *((uint32_t*)file_buffer);
    if (magic != MODEL_FILE_MAGIC) {
        fprintf(stderr, "Invaild model file: Header magic error.\n");
        free(file_buffer);
        return NULL;
    }
    size  = *((uint32_t*)(file_buffer + 4));
    if (file_size != size) {
        fprintf(stderr, "Invaild model file: File size error.\n");
        free(file_buffer);
        return NULL;
    }

    return file_buffer;
}

isr_model_t * model_detect(char * model)
{
    int model_index;
    int check_result;

    model_index = 0;

    while (isr_models[model_index] != NULL) {
        check_result = isr_models[model_index]->check(model);

        if (check_result == 0) {
            fprintf(stderr, "Model: %s detected.\n", isr_models[model_index]->name);
            return isr_models[model_index];
        }

        model_index ++;
    }
    return NULL;
}
