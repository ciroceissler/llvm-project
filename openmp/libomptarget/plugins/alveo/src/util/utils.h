/*
 * utils.h
 *
 *  Created on: Jun 27, 2018
 *      Author: ramon
 */


#ifndef SRC_SDX_RTL_KERNEL_WIZARD_SDX_KERNEL_WIZARD_0_EX_SDX_IMPORTS_UTILS_H_
#define SRC_SDX_RTL_KERNEL_WIZARD_SDX_KERNEL_WIZARD_0_EX_SDX_IMPORTS_UTILS_H_

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
// #include <CL/opencl.h>
#include <vector>

#define NUM_WORKGROUPS (1)
#define WORKGROUP_SIZE (256)
#define MAX_LENGTH 8192

#if defined(SDX_PLATFORM) && !defined(TARGET_DEVICE)
#define STR_VALUE(arg)      #arg
#define GET_STRING(name) STR_VALUE(name)
#define TARGET_DEVICE GET_STRING(SDX_PLATFORM)
#endif

// extern int err;                            // error code returned from api calls
// extern int check_status;// = 0;
// extern const uint number_of_words;// = 4096; // 16KB of data
//
//
// extern cl_platform_id platform_id;         // platform id
// extern cl_device_id device_id;             // compute device id
// extern cl_context context;                 // compute context
// extern cl_command_queue commands;          // compute command queue
// extern cl_program program;                 // compute programs
// extern cl_kernel kernel;                   // compute kernel
//
// extern char cl_platform_vendor[1001];
// extern char target_device_name[1001];// = TARGET_DEVICE;
//
// extern int h_axi00_ptr0_input[MAX_LENGTH];                    // host memory for input vector
// extern int h_axi00_ptr0_output[MAX_LENGTH];                   // host memory for output vector
// extern cl_mem d_axi00_ptr0;
// extern char *xclbin;

void init_util();
void* data_alloc(int size);
void data_submit(void *tgt_ptr, void *hst_ptr, int64_t size);
void run_target();
void data_retrieve(void *hst_ptr, void *tgt_ptr, int size);
void data_delete(void *tgt_ptr);
void cleanup();

#endif /* SRC_SDX_RTL_KERNEL_WIZARD_SDX_KERNEL_WIZARD_0_EX_SDX_IMPORTS_UTILS_H_ */
