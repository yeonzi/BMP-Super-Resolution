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
#include <core/image_utils.h>
#include <contrib/compute/opencl.h>
#include <models/models.h>

#define NEWLINE "\n"

void show_usage(void);
int read_parameter(int argc, char const *argv[]);

const char * input_file = NULL;
const char * output_file = NULL;
const char * model_selected = NULL;
const char   model_default[] = "basic";
char * model_content;
unsigned int device_id = 0;

int main(int argc, char const *argv[])
{

    image_t * img1;
    image_t * img2;
    image_t * img3;

    int ret;

    ret = read_parameter(argc,argv);

    if (ret <= 0) {
        return ret;
    }

    opencl_init(device_id);

	img1 = image_open(input_file);

	if (img1 == NULL) {
		fprintf(stderr, "Cannot open file.\n");
		return -1;
	}

	img2 = image_make_border(img1, 10);
	img3 = image_chop_border(img2, 10);

	return image_save(img3, output_file, IMG_FMT_WINBMP);
}

void show_usage(void) {
    puts("Usage: image_2x [options]" NEWLINE
        "Options:" NEWLINE
        "    -h              Show this usage and exit." NEWLINE
        "    -i <file_name>  Specific input file." NEWLINE
        "                    Supported format:" NEWLINE
        "                        Netpbm ASCII Portable PixMap (.ppm)" NEWLINE
        "                        BGR-24 Bitmap Image File (.bmp)" NEWLINE
        "    -l              List all platform for caculate and exit." NEWLINE
        "    -m <model>      Specific model file. (not available now)" NEWLINE
        "    -o <file_name>  Specific output file." NEWLINE
        "                    Supported format:" NEWLINE
        "                        BGR-24 Bitmap Image File (.bmp)" NEWLINE
        "    -p <device ID>  Specific caculate platform. (Optional)" NEWLINE
        "                    Platform available can be listed by -l." NEWLINE
        "                    Run without -p or invalid id will use native" NEWLINE
        "                    code to run (CPU single thread, very slow)." NEWLINE
        "    -v              Show Version. (not available now)");
}

int read_parameter(int argc, char const *argv[])
{
    const char * p;
    int         i;

    for (i = 1; i < argc; i++) {
        p = argv[i];

        /* only support args start with '-' */
        if (*p++ != '-') {
            printf("Invalid option: \"%s\"", argv[i]);
            show_usage();
            return -1;
        }

        while (*p) {
            switch (*p++) {
                case '?':
                case 'h':
                    show_usage();
                    return 0;
                case 'i':
                    if (*p) {
                        input_file = p;
                        goto next;
                    } else if (argv[++i]) {
                        input_file = argv[i];
                        goto next;
                    } else {
                        printf("option \"-i\" requires parameter\n");
                        return -1;
                    }
                case 'o':
                    if (*p) {
                        output_file = p;
                        goto next;
                    } else if (argv[++i]) {
                        output_file = argv[i];
                        goto next;
                    } else {
                        printf("option \"-o\" requires parameter\n");
                        return -1;
                    }
                case 'm':
                    if (*p) {
                        model_selected = p;
                        goto next;
                    } else if (argv[++i]) {
                        model_selected = argv[i];
                        goto next;
                    } else {
                        printf("option \"-m\" requires parameter\n");
                        return -1;
                    }
                case 'p':
                    if (*p) {
                        device_id = atoi(p);
                        goto next;
                    }else if (argv[++i]) {
                        device_id = atoi(argv[i]);
                        goto next;
                    }else{
                        printf("option \"-p\" requires parameter\n");
                        return -1;
                    }
                case 'l':
                    opencl_list();
                    return 0;
                default:
                    printf("invalid option: \"%c\"\n", *(p - 1));

                    return -1;
            }
        }
        next:;
    }

    if ( input_file == NULL ) {
        fprintf(stderr, "Parameter Error: no input file\n");
        show_usage();
        return -1;
    }
    if ( output_file == NULL ) {
        fprintf(stderr, "Parameter Error: no output file\n");
        show_usage();
        return -1;
    }
    if ( model_selected == NULL ) {
        model_selected = model_default;
    } else {
        model_content = model_read(model_selected);
        if (model_content == NULL) {
            fprintf(stderr, "Cannot not model %s\n", model_selected);
            return -1;
        }
    }

    return 1;
}
