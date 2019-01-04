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

#include "opencl.h"

#ifdef __APPLE__    
#include <OpenCL/cl.h>    
#else    
#include <CL/cl.h>    
#endif

#include <stdio.h>

#define MAX_PLATFORMS 4
#define MAX_DEVICES   8

int opencl_list(void)
{
    cl_int              err;

    cl_platform_id      platforms[MAX_PLATFORMS];
    cl_uint             platform_cnt;
    unsigned int        platform_index;

    cl_device_id        devices[MAX_PLATFORMS][MAX_DEVICES];
    cl_uint             device_cnt[MAX_PLATFORMS];
    unsigned int        device_index;

    char vendor[100];
    char name[100];

    int cnt = 0;

    /* Identify a platform */
    err = clGetPlatformIDs(MAX_PLATFORMS, platforms, &platform_cnt);

    if(platform_cnt <= 0) {
        fprintf(stderr, "No OpenCL Platform Available.\n");
      return 0;
    } else {
        fprintf(stderr, "%d OpenCL Platform Found.\n", platform_cnt);
    }

    for (platform_index = 0; platform_index < platform_cnt; platform_index++) {
    	err = clGetDeviceIDs( platforms[platform_index], \
                              CL_DEVICE_TYPE_ALL, 5, \
                              devices[platform_index], \
                              &device_cnt[platform_index] );
    }

    for (platform_index = 0; platform_index < platform_cnt; platform_index++) {
        err = clGetPlatformInfo( platforms[platform_index], CL_PLATFORM_VENDOR, sizeof(vendor), vendor ,NULL);
        err = clGetPlatformInfo( platforms[platform_index], CL_PLATFORM_NAME, sizeof(name), name ,NULL);
        fprintf(stderr, "Platform %d: %s %s (%d Devices)\n", platform_index, \
            vendor, name, device_cnt[platform_index]);
    }

    fprintf(stderr, "Device 0: Native code (without any accelerator)\n");

    for (platform_index = 0; platform_index < platform_cnt; platform_index++) {
        for (device_index = 0; device_index < device_cnt[platform_index]; device_index++) {
        	cnt ++;
            err = clGetDeviceInfo( devices[platform_index][device_index], \
            			CL_DEVICE_VENDOR, sizeof(vendor), vendor ,NULL);
            err = clGetDeviceInfo( devices[platform_index][device_index], \
            			CL_DEVICE_NAME, sizeof(name), name ,NULL);
            fprintf(stderr, "Device %d: %s %s (on OpenCL Platfrom %d)\n", \
            	cnt, vendor, name, platform_index);
        }
    }

    return 0;
}