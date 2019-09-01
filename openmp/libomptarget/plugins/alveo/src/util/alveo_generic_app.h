//===--- opae_generic_app.h - AFU Link Interface Implementation --- C++ -*-===//
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

#include <string>
#include <cstring>
#include <deque>
#include <unistd.h>

#define CTL_ASSERT_RST            0
#define CTL_DEASSERT_RST          1
#define CTL_START                 3
#define CTL_STOP                  7

// csr - memory map
#define CSR_DSM_BASEL        0x0110
#define CSR_DSM_BASEH        0x0114
#define CSR_CTL              0x0118
#define CSR_BASE_BUFFER      0x0120

struct Buffer {
  uint64_t size;
  uint64_t phys;
  void*    virt;
};

class AlveoGenericApp{
private:
  std::deque<Buffer> buffers;

public:

  AlveoGenericApp();
  ~AlveoGenericApp();

  int init();
  int finish();
  void* alloc_buffer(uint64_t size);
  void delete_buffer(void *tgt_ptr);
  void submit_buffer(void *tgt_ptr, void *hst_ptr, int64_t size);
  void retrieve_buffer(void *hst_ptr, void *tgt_ptr, int64_t size);


  int run();    ///< Return 0 if success
  int program(const char *module);
};
