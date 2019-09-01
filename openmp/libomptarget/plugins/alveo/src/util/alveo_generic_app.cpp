//===--- opae_generic_app.cpp - AFU Link Interface Implementation - C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// Generic Link Interface implementation
//
//===----------------------------------------------------------------------===//

#include "alveo_generic_app.h"
#include "utils.h"

int h_axi00_ptr0_input[MAX_LENGTH];                    // host memory for input vector
int h_axi00_ptr0_output[MAX_LENGTH];

char *xclbin;

AlveoGenericApp::AlveoGenericApp() {}

AlveoGenericApp::~AlveoGenericApp() {}

int AlveoGenericApp::init() {
  
  init_util();
  return 0;
}

void* AlveoGenericApp::alloc_buffer(uint64_t size) {
  return data_alloc(size);

}

void AlveoGenericApp::submit_buffer(void *tgt_ptr, void *hst_ptr, int64_t size) {
  data_submit(tgt_ptr, hst_ptr, size);

}


void AlveoGenericApp::retrieve_buffer(void *hst_ptr, void *tgt_ptr, int64_t size){
  data_retrieve(hst_ptr, tgt_ptr, size);

}

void AlveoGenericApp::delete_buffer(void *tgt_ptr) {
  data_delete(tgt_ptr);
  return;
}

int AlveoGenericApp::finish() {

  return 0;
}

int AlveoGenericApp::program(const char *module) {

  return 0;
}

int AlveoGenericApp::run() {
    run_target();

    return 0;
}
