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

#include "image.h"
#include <stdio.h>

#ifdef __APPLE__    
#include <OpenCL/cl.h>    
#else    
#include <CL/cl.h>    
#endif

#define image_pixel(image,x,y)  &((float *)image->data)[3 * ((y) * image->width + (x))]

image_t * image_conv(image_t * src, image_t * kernel)
{
    image_t * border;
    image_t * conved;
    image_t * choped;

    size_t  border_size;
    int32_t x0;
    int32_t y0;
    int32_t x1;
    int32_t y1;
    int32_t dx;
    int32_t dy;
    float   pixel_tmp[3];

    float * img;
    float * kern;

    if (kernel->model != IMG_MODEL_BGR && kernel->model != IMG_MODEL_YCBCR) {
        fprintf(stderr, "RGB or YCbCr is the only color model supported by conv operation\n");
        return NULL;
    }

    if (kernel->width%2!=1||kernel->height%2!=1) {
        fprintf(stderr, "Conv need a odd kernel\n");
        return NULL;
    }

    dx = kernel->width / 2 + 1;
    dy = kernel->height / 2 + 1;

    border_size = kernel->width>kernel->height?kernel->width:kernel->height;

    border = image_make_border(src, border_size);
    image_convert(border, kernel->model);
    conved = image_new(border->width, border->height, kernel->model);

    for(x0 = 0; x0 < border->width - kernel->width; x0++){
        for(y0 = 0; y0 < border->height - kernel->height; y0++){
            /* ergodic pixel in image */
            pixel_tmp[0] = 0;
            pixel_tmp[1] = 0;
            pixel_tmp[2] = 0;
            for(x1 = 0; x1 < kernel->width; x1++){
                for(y1 = 0; y1 < kernel->height; y1++){
                    img  = image_pixel(border, x0+x1, y0+y1);
                    kern = image_pixel(kernel, x1, y1);
                    pixel_tmp[0] += kern[0] * img[0];
                    pixel_tmp[1] += kern[1] * img[1];
                    pixel_tmp[2] += kern[2] * img[2];
                }
            }
            pixel_tmp[0] = pixel_tmp[0] / kernel->div + kernel->bias;
            pixel_tmp[1] = pixel_tmp[1] / kernel->div + kernel->bias;
            pixel_tmp[2] = pixel_tmp[2] / kernel->div + kernel->bias;

            img  = image_pixel(conved, x0+dx, y0+dy);
            img[0] = pixel_tmp[0];
            img[1] = pixel_tmp[1];
            img[2] = pixel_tmp[2];
            
        }
    }

    choped = image_chop_border(conved, border_size);

    image_free(border);

    return choped;
}

image_t * image_conv_raw(image_t * src, image_t * kernel)
{
    image_t * conved;    

    size_t    border_size;
    int32_t   x0;
    int32_t   y0;
    int32_t   x1;
    int32_t   y1;
    int32_t   dx;
    int32_t   dy;
    float   pixel_tmp[3];
    float * img;
    float * kern;

    dx = kernel->width / 2;
    dy = kernel->height / 2;

    border_size = kernel->width>kernel->height?kernel->width:kernel->height;

    image_convert(src, kernel->model);

    conved = image_new(src->width, src->height, kernel->model);

    for(x0 = 0; x0 < src->width - kernel->width; x0++){
        for(y0 = 0; y0 < src->height - kernel->height; y0++){
            /* ergodic pixel in image */
            pixel_tmp[0] = 0;
            pixel_tmp[1] = 0;
            pixel_tmp[2] = 0;
            for(x1 = 0; x1 < kernel->width; x1++){
                for(y1 = 0; y1 < kernel->height; y1++){
                    img  = image_pixel(src, x0+x1, y0+y1);
                    kern = image_pixel(kernel, x1, y1);
                    pixel_tmp[0] += kern[0] * img[0];
                    pixel_tmp[1] += kern[1] * img[1];
                    pixel_tmp[2] += kern[2] * img[2];
                }
            }
            pixel_tmp[0] = pixel_tmp[0] / kernel->div + kernel->bias;
            pixel_tmp[1] = pixel_tmp[1] / kernel->div + kernel->bias;
            pixel_tmp[2] = pixel_tmp[2] / kernel->div + kernel->bias;

            img  = image_pixel(conved, x0+dx, y0+dy);
            img[0] = pixel_tmp[0];
            img[1] = pixel_tmp[1];
            img[2] = pixel_tmp[2];
            
        }
    }

    return conved;
}

image_t * kernel_load(const char * file_name)
{
    image_t * img;

    int  num;
    int  width;
    int  height;

    float * ptr;
    float * ptr_end;

    FILE * fp;

    fp = fopen(file_name, "rb");

    /* Read Magic */
    fscanf(fp, "P%d", &num);

    if (num != 8) return NULL;

    /* Read info */
    fscanf(fp, "%d %d", &width, &height);

    fscanf(fp, "%d", &num);
    if (num == 255) {
        img = image_new(width, height, IMG_MODEL_BGR);
    } else if (num == 256) {
        img = image_new(width, height, IMG_MODEL_YCBCR);
    } else {
        return NULL;
    }

    ptr = img->data;
    ptr_end = &ptr[3 * width * height];
    fscanf(fp, "%f %f", &img->bias, &img->div);
    while (ptr < ptr_end) {
        fscanf(fp, "%f", ptr);
        ptr ++;
    }

    fclose(fp);

    return img;
}

/* Find a GPU or CPU associated with the first available platform */
cl_device_id select_device(void)
{
    int             err;
    char            buf[100];
    cl_device_id    device;
    cl_platform_id  platform;

    /* Identify a platform */
    err = clGetPlatformIDs(1, &platform, NULL);
    if(err < 0) {
      perror("Couldn't identify a platform");
      exit(-1);
    } 

    /* Access a device */
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 1, &device, NULL);

    if(err == CL_DEVICE_NOT_FOUND) {
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, NULL);
    }

    if(err < 0) {
      perror("Couldn't access any devices");
      exit(-1);   
    }

    /* Print device info */
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(buf), buf ,NULL);
    printf("\r\033[KDevice %s ", buf);
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(buf), buf ,NULL);
    printf("%s selected\n", buf);

    return device;
}


/* Create program from a file and compile it */
cl_program build_program(cl_context content, cl_device_id devices) {

    cl_program program;
    FILE    *program_handle;
    static size_t   program_size = 0;
    static char    *program_buffer;   
    size_t   log_size;
    char    *program_log;
    int      err;
    // extern char  *program_binary;
    // extern size_t binary_size;
    extern char program_code[];
    extern size_t code_size;

    if (program_size == 0) {
        /* Read program file and place content into buffer */
        program_handle = fopen("./image_conv.cl", "r");
        if(program_handle == NULL) {
            perror("Couldn't find the program file");
            exit(1);
        }
        
        /* Get program file size */
        fseek(program_handle, 0, SEEK_END);
        program_size = ftell(program_handle);
        rewind(program_handle);

        /* Load program */
        program_buffer = malloc(program_size + 1);
        program_buffer[program_size] = '\0';
        fread(program_buffer, sizeof(char), program_size, program_handle);
        fclose(program_handle);
    }

    /* Create program from file */
    program = clCreateProgramWithSource(content, 1, (const char**)&program_buffer, &program_size, &err);
    if(err < 0) {
        perror("Couldn't create the program");
        exit(1);
    }

    /* Build program */
    err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
    if(err < 0) {
        /* Find size of log and print to std output */
        clGetProgramBuildInfo(program, devices, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
        program_log = malloc(log_size + 1);
        program_log[log_size] = '\0';
        clGetProgramBuildInfo(program, devices, CL_PROGRAM_BUILD_LOG, log_size + 1, program_log, NULL);
        printf("%s\n", program_log);
        free(program_log);
        exit(1);
    }

    // clGetProgramInfo(program, CL_PROGRAM_BINARY_SIZES, 0, NULL, &binary_size);
    // program_binary = malloc(binary_size + 1);
    // program_binary[binary_size] = '\0';
    // clGetProgramInfo(program, CL_PROGRAM_BINARIES, binary_size + 1, program_binary, NULL);

    return program;
}

image_t * opencl_conv(image_t * src, image_t * kern)
{
    static cl_device_id  device;
    extern char         *program_binary;
    extern size_t        binary_size;
    static int           opencl_inited;

    cl_int          err;
    cl_context      context;
    cl_program      program;
    cl_kernel       kernel;
    cl_mem          src_buf;
    cl_mem          kern_buf;
    cl_mem          out_buf;
    image_t        *conved;


    cl_command_queue queue;

    size_t  border_size;
    size_t  offset[2];
    size_t  work_size[2];
    int32_t dx;
    int32_t dy;


    dx = kern->width / 2;
    dy = kern->height / 2;

    border_size = kern->width>kern->height?kern->width:kern->height;

    image_convert(src, kern->model);
    conved = image_new(src->width, src->height, kern->model);

    offset[0] = 0;
    offset[1] = 0;

    work_size[0] = src->width - kern->width;
    work_size[1] = src->height - kern->height;

    if (!opencl_inited) {
        device  = select_device();
        opencl_inited = 1;
        if(err < 0) {
            perror("Couldn't create a kernel");
            exit(1);
        }
    }

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    program = build_program(context, device);
    kernel  = clCreateKernel(program, "conv", &err);

    src_buf   = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 3 * src->width * src->height * sizeof(float), src->data, &err);
    kern_buf  = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 3 * kern->width * kern->height * sizeof(float), kern->data, &err);
    out_buf  = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 3 * src->width * src->height * sizeof(float), conved->data, &err);

    if(err < 0) {
        perror("Couldn't create a buffer");
        exit(-1);   
    }

    queue = clCreateCommandQueue(context, device, 0, &err);
    if(err < 0) {
        perror("Couldn't create a command queue");
        exit(1);   
    }

    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &src_buf);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &kern_buf);
    err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &out_buf);
    err |= clSetKernelArg(kernel, 3, sizeof(int), &src->width);
    err |= clSetKernelArg(kernel, 4, sizeof(int), &kern->width);
    err |= clSetKernelArg(kernel, 5, sizeof(int), &kern->height);
    err |= clSetKernelArg(kernel, 6, sizeof(int), &dx);
    err |= clSetKernelArg(kernel, 7, sizeof(int), &dy);
    err |= clSetKernelArg(kernel, 8, sizeof(float), &kern->bias);
    err |= clSetKernelArg(kernel, 9, sizeof(float), &kern->div);
    if(err < 0) {
        perror("Couldn't create a kernel argument");
        exit(1);
    }

    err = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, work_size, NULL, 0, NULL, NULL);
    if(err < 0) {
        perror("Couldn't enqueue the kernel");
        exit(1);
    }

    err = clEnqueueReadBuffer(queue, out_buf, CL_TRUE, 0, 3 * conved->width * conved->height * sizeof(float), conved->data, 0, NULL, NULL); 
    if(err < 0) {
        perror("Couldn't read the buffer");
        exit(1);
    }

    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseMemObject(src_buf);
    clReleaseMemObject(kern_buf);
    clReleaseMemObject(out_buf);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return conved;
}
