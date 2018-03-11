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


#include <stdio.h>
#include <stdlib.h>

#ifdef __APPLE__    
#include <OpenCL/cl.h>    
#else    
#include <CL/cl.h>    
#endif

#include "bmp.h"
#include "image.h"
#include "image_conv.h"

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
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

    if(err == CL_DEVICE_NOT_FOUND) {
      err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_DEFAULT, 1, &device, NULL);
    }

    if(err < 0) {
      perror("Couldn't access any devices");
      exit(-1);   
    }

    /* Print device info */
    clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(buf), buf ,NULL);
    printf("Device %s ", buf);
    clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(buf), buf ,NULL);
    printf("%s selected\n", buf);

    return device;
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
   int err;

   /* Read program file and place content into buffer */
   program_handle = fopen(filename, "r");
   if(program_handle == NULL) {
      perror("Couldn't find the program file");
      exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);

   /* Create program from file */
   program = clCreateProgramWithSource(ctx, 1, 
      (const char**)&program_buffer, &program_size, &err);
   if(err < 0) {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   /* Build program */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
      program_log[log_size] = '\0';
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            log_size + 1, program_log, NULL);
      printf("%s\n", program_log);
      free(program_log);
      exit(1);
   }

   return program;
}

cl_device_id    device;
cl_context      context;
cl_program      program;
cl_int          err;
cl_kernel       kernel;

int open_inited = 0;

image_t * opencl_conv(image_t * src, image_t * kern)
{
    extern cl_device_id    device;
    extern cl_context      context;
    extern cl_program      program;
    extern cl_kernel       kernel;
    extern cl_int          err;
    extern int open_inited;

    image_t * border;
    image_t * conved;
    image_t * choped;

    size_t  border_size;
    size_t  offset[2];
    size_t  work_size[2];
    int32_t dx;
    int32_t dy;

    cl_mem src_buf;
    cl_mem kern_buf;
    cl_mem out_buf;

    cl_command_queue queue;

    dx = kern->width / 2 + 1;
    dy = kern->height / 2 + 1;

    border_size = kern->width>kern->height?kern->width:kern->height;

    border = image_make_border(src, border_size);
    image_convert(border, kern->model);
    conved = image_new(border->width, border->height, kern->model);

    offset[0] = 0;
    offset[1] = 0;

    work_size[0] = border->width;
    work_size[1] = border->height;

    if (!open_inited) {
        device  = select_device();
        context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
        program = build_program(context, device, "./image_conv.cl");
        kernel  = clCreateKernel(program, "conv", &err);

        if(err < 0) {
            perror("Couldn't create a kernel");
            exit(1);
        }
    }

    src_buf   = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 3 * border->width * border->height * sizeof(float), border->data, &err);
    kern_buf  = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, 3 * kern->width * kern->height * sizeof(float), kern->data, &err);
    out_buf  = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, 3 * border->width * border->height * sizeof(float), conved->data, &err);

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
    err |= clSetKernelArg(kernel, 3, sizeof(int), &border->width);
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

    err = clEnqueueNDRangeKernel(queue, kernel, 2, offset, work_size, NULL, 0, NULL, NULL);
    if(err < 0) {
        perror("Couldn't enqueue the kernel");
        exit(1);
    }

    err = clEnqueueReadBuffer(queue, out_buf, CL_TRUE, 0, 3 * border->width * border->height * sizeof(float), conved->data, 0, NULL, NULL); 
    if(err < 0) {
        perror("Couldn't read the buffer");
        exit(1);
    }

    choped = image_chop_border(conved, border_size);

    image_free(border);
    clReleaseMemObject(src_buf);
    clReleaseMemObject(kern_buf);
    clReleaseMemObject(out_buf);

    return choped;
}

int main(void){
    image_t * image;
    image_t * kernel;
    image_t * conv;

    image = bmp_load("./test.bmp");
    kernel = kernel_load("./test.kern");
    conv = opencl_conv(image, kernel);
    image_gray(conv);
    bmp_save(conv,"./out.bmp");
    return 0;
}