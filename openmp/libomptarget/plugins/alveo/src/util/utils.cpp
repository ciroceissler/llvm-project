/*
 * utils.c
 *
 *  Created on: Jun 27, 2018
 *      Author: ramon
 */
#include "utils.h"

void init_util(){} ;
void* data_alloc(int size){} ;
void data_submit(void *tgt_ptr, void *hst_ptr, int64_t size){} ;
void run_target(){} ;
void data_retrieve(void *hst_ptr, void *tgt_ptr, int size){} ;
void data_delete(void *tgt_ptr){} ;
void cleanup(){} ;

// int err;                            // error code returned from api calls
// int check_status;// = 0;
// const uint number_of_words = 4096; // 16KB of data
//
//
// cl_platform_id platform_id;         // platform id
// cl_device_id device_id;             // compute device id
// cl_context context;                 // compute context
// cl_command_queue commands;          // compute command queue
// cl_program program;                 // compute programs
// cl_kernel kernel;                   // compute kernel
//
// char cl_platform_vendor[1001];
// char target_device_name[1001] = TARGET_DEVICE;
// cl_mem d_axi00_ptr0;
// int KernelArgCount = 0;
//
// std::vector<cl_uint> scalars_values; // buffers sizes
// std::vector<cl_mem> clmem_ptrs; // buffers pointers
//
//
//
// int load_file_to_memory(const char *filename, char **result)
// {
//   uint size = 0;
//   FILE *f = fopen(filename, "rb");
//   if (f == NULL) {
//     *result = NULL;
//     return -1; // -1 means file opening fail
//   }
//   fseek(f, 0, SEEK_END);
//   size = ftell(f);
//   fseek(f, 0, SEEK_SET);
//   *result = (char *)malloc(size+1);
//   if (size != fread(*result, sizeof(char), size, f)) {
//     free(*result);
//     return -2; // -2 means file reading fail
//   }
//   fclose(f);
//   (*result)[size] = 0;
//   return size;
// }
//
//
// void init_util()
// {
//   // Get all platforms and then select Xilinx platform
//   cl_platform_id platforms[16];       // platform id
//   cl_uint platform_count;
//   int platform_found = 0;
//   err = clGetPlatformIDs(16, platforms, &platform_count);
//   if (err != CL_SUCCESS) {
//     printf("Error: Failed to find an OpenCL platform!\n");
//     printf("Test failed\n");
//     //    return EXIT_FAILURE;
//   }
//   printf("INFO: Found %d platforms\n", platform_count);
//
//   // Find Xilinx Plaftorm
//   for (unsigned int iplat=0; iplat<platform_count; iplat++) {
//     err = clGetPlatformInfo(platforms[iplat], CL_PLATFORM_VENDOR, 1000, (void *)cl_platform_vendor,NULL);
//     if (err != CL_SUCCESS) {
//       printf("Error: clGetPlatformInfo(CL_PLATFORM_VENDOR) failed!\n");
//       printf("Test failed\n");
//       //        return EXIT_FAILURE;
//     }
//     if (strcmp(cl_platform_vendor, "Xilinx") == 0) {
//       printf("INFO: Selected platform %d from %s\n", iplat, cl_platform_vendor);
//       platform_id = platforms[iplat];
//       platform_found = 1;
//     }
//   }
//   if (!platform_found) {
//     printf("ERROR: Platform Xilinx not found. Exit.\n");
//     //    return EXIT_FAILURE;
//   }
//
//   // Get Accelerator compute device
//   cl_uint num_devices;
//   unsigned int device_found = 0;
//   cl_device_id devices[16];  // compute device id
//   char cl_device_name[1001];
//   err = clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_ACCELERATOR, 16, devices, &num_devices);
//   printf("INFO: Found %d devices\n", num_devices);
//   if (err != CL_SUCCESS) {
//     printf("ERROR: Failed to create a device group!\n");
//     printf("ERROR: Test failed\n");
//     //    return -1;
//   }
//
//   //iterate all devices to select the target device.
//   for (uint i=0; i<num_devices; i++) {
//     err = clGetDeviceInfo(devices[i], CL_DEVICE_NAME, 1024, cl_device_name, 0);
//     if (err != CL_SUCCESS) {
//       printf("Error: Failed to get device name for device %d!\n", i);
//       printf("Test failed\n");
//       //    return EXIT_FAILURE;
//     }
//     printf("CL_DEVICE_NAME %s\n", cl_device_name);
//     if(strcmp(cl_device_name, target_device_name) == 0) {
//       device_id = devices[i];
//       device_found = 1;
//       printf("Selected %s as the target device\n", cl_device_name);
//     }
//   }
//
//   if (!device_found) {
//     printf("Target device %s not found. Exit.\n", target_device_name);
//     //    return EXIT_FAILURE;
//   }
//
//   // Create a compute context
//   //
//   context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
//   if (!context) {
//     printf("Error: Failed to create a compute context!\n");
//     printf("Test failed\n");
//     //    return EXIT_FAILURE;
//   }
//
//   // Create a command commands
//   commands = clCreateCommandQueue(context, device_id, 0, &err);
//   if (!commands) {
//     printf("Error: Failed to create a command commands!\n");
//     printf("Error: code %i\n",err);
//     printf("Test failed\n");
//     //   return EXIT_FAILURE;
//   }
//
//   int status;
//
//   // Create Program Objects
//   // Load binary from disk
//   unsigned char *kernelbinary;
//   //      char *xclbin = argv[1];
//
//   //------------------------------------------------------------------------------
//   // xclbin
//   //------------------------------------------------------------------------------
//   printf("INFO: loading xclbin %s\n", "binary_container_1.xclbin");
//   int n_i0 = load_file_to_memory("binary_container_1.xclbin", (char **) &kernelbinary);
//   if (n_i0 < 0) {
//     printf("failed to load kernel from xclbin: %s\n", xclbin);
//     printf("Test failed\n");
//     //    return EXIT_FAILURE;
//   }
//
//   size_t n0 = n_i0;
//
//   // Create the compute program from offline
//   program = clCreateProgramWithBinary(context, 1, &device_id, &n0,
//       (const unsigned char **) &kernelbinary, &status, &err);
//
//   if ((!program) || (err!=CL_SUCCESS)) {
//     printf("Error: Failed to create compute program from binary %d!\n", err);
//     printf("Test failed\n");
//     //    return EXIT_FAILURE;
//   }
//
//   // Build the program executable
//   //
//   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
//   if (err != CL_SUCCESS) {
//     size_t len;
//     char buffer[2048];
//
//     printf("Error: Failed to build program executable!\n");
//     clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
//     printf("%s\n", buffer);
//     printf("Test failed\n");
//     //    return EXIT_FAILURE;
//   }
//
//   // Create the compute kernel in the program we wish to run
//   //
//   kernel = clCreateKernel(program, "image_filters", &err);
//   if (!kernel || err != CL_SUCCESS) {
//     printf("Error: Failed to create compute kernel!\n");
//     printf("Test failed\n");
//     //    return EXIT_FAILURE;
//   }
//
// }
//
// void* data_alloc(int size)
// {
//   cl_mem dt = clCreateBuffer(context,  CL_MEM_READ_WRITE,  size, NULL, NULL);
//   if (!(dt)) {
//     printf("Error: Failed to allocate device memory!\n");
//     printf("Test failed\n");
//     // return EXIT_FAILURE;
//   }
//
//   clmem_ptrs.push_back(dt);
//   scalars_values.push_back((cl_uint)size);
//
//   return (void*) dt;
// }
//
// void data_submit(void *tgt_ptr, void *hst_ptr, int64_t size)
// {
//   //err = clEnqueueWriteBuffer(commands, d_axi00_ptr0, CL_TRUE, 0, sizeof(int) * number_of_words, h_axi00_ptr0_input, 0, NULL, NULL);
//   err = clEnqueueWriteBuffer(commands, (cl_mem)tgt_ptr, CL_TRUE, 0, size, hst_ptr, 0, NULL, NULL);
//   if (err != CL_SUCCESS) {
//     printf("Error: Failed to write to source array h_axi00_ptr0_input!\n");
//     printf("Test failed\n");
//
//     //  return EXIT_FAILURE;
//   }
// }
//
//
// void run_target()
// {
//   err = 0;
//   int args_c = 0;  // contador de argumentos
//
//   for(auto scl: scalars_values){
//     err = clSetKernelArg(kernel, args_c, sizeof(cl_uint),&scl);
//     if(err != CL_SUCCESS){
//       printf("Error: Failed to set kernel arguments! %d\n", err);
//       printf("Test failed\n");
//     }
//     args_c++;
//   }
//
//   for(auto ptrs: clmem_ptrs){
//     err = clSetKernelArg(kernel, args_c, sizeof(cl_mem), &ptrs);
//     if(err != CL_SUCCESS){
//       printf("Error: Failed to set kernel arguments! %d\n", err);
//       printf("Test failed\n");
//     }
//     args_c++;
//   }
//
//   // Execute the kernel over the entire range of our 1d input data set
//   // using the maximum number of work group items for this device
//   clFinish(commands);
//   err = clEnqueueTask(commands, kernel, 0, NULL, NULL);
//   if (err) {
//     printf("Error: Failed to execute kernel! %d\n", err);
//     printf("Test failed\n");
//
//     // return EXIT_FAILURE;
//   }
// }
//
// void data_retrieve(void *hst_ptr, void *tgt_ptr, int size)
// {
//   cl_event readevent;
//   clFinish(commands);
//
//   err = 0;
//   err |= clEnqueueReadBuffer( commands, (cl_mem )tgt_ptr, CL_TRUE, 0, size, hst_ptr, 0, NULL, &readevent );
//
//   if (err != CL_SUCCESS) {
//     printf("Error: Failed to read output array! %d\n", err);
//     printf("Test failed\n");
//
//     // return EXIT_FAILURE;
//   }
//
//   clWaitForEvents(1, &readevent);
//   // Check Results
// }
//
// void data_delete(void *tgt_ptr){
//   printf("%s\n", "UTILS INFO: calling data_delete \n");
//   clReleaseMemObject((cl_mem)tgt_ptr);
// }
//
//
// void cleanup()
// {
//   printf("%s\n", "UTILS INFO: calling cleanup \n");
//   clReleaseProgram(program);
//   clReleaseKernel(kernel);
//   clReleaseCommandQueue(commands);
//   clReleaseContext(context);
//
//   if (check_status) {
//     printf("INFO: Test failed\n");
//
//     // return EXIT_FAILURE;
//   } else {
//     printf("INFO: Test completed successfully.\n");
//   }
//
// }
