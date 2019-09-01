//===----RTLs/alveo/src/rtl.cpp - Target RTLs Implementation ------ C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// RTL for ALVEO machine
//
//===----------------------------------------------------------------------===//

#include <cassert>
#include <cstddef>
#include <list>
#include <string>
#include <vector>
#include <ffi.h>
#include <gelf.h>
#include <link.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "util/alveo_generic_app.h"

#include "omptarget.h"

#ifndef TARGET_NAME
#define TARGET_NAME ALVEO
#endif

#define GETNAME2(name) #name
#define GETNAME(name) GETNAME2(name)
#define DP(...) DEBUGP("Target " GETNAME(TARGET_NAME) " RTL", __VA_ARGS__)

#include "../../common/elf_common.c"

#define NUMBER_OF_DEVICES 1
#define OFFLOADSECTIONNAME ".omp_offloading.entries"

// Utility for retrieving and printing ALVEO error string.
#ifdef ALVEO_ERROR_REPORT
#define ALVEO_ERR_STRING(err)                                              \
  do {                                                                     \
    const char *errStr;                                                    \
    cuGetErrorString(err, &errStr);                                        \
    DP("ALVEO error is: %s\n", errStr);                                    \
  } while (0)
#else
#define ALVEO_ERR_STRING(err)                                              \
  {}
#endif

#ifdef OMPTARGET_DEBUG
  #define TIME_START     t_start = rtclock();
  #define TIME_PRINT(x)  t_end = rtclock(); printf("[time][alveo] %s = %0.6lfs\n", x, t_end - t_start);
  double rtclock() {
    struct timezone Tzp;
    struct timeval Tp;
    int stat;
    stat = gettimeofday (&Tp, &Tzp);
    if (stat != 0) printf("Error return from gettimeofday: %d",stat);
    return(Tp.tv_sec + Tp.tv_usec*1.0e-6);
  }

  double t_start, t_end;
#else
  #define TIME_START
  #define TIME_PRINT(x)
#endif  //  OMPTARGET_DEBUG

/// Array of Dynamic libraries loaded for this target.
struct DynLibTy {
  char *FileName;
  void *Handle;
};

/// Keep entries table per device.
struct FuncOrGblEntryTy {
  __tgt_target_table Table;
};

/// Class containing all the device information.
class RTLDeviceInfoTy {
  std::vector<FuncOrGblEntryTy> FuncGblEntries;

public:

  std::list<DynLibTy> DynLibs;

  // Record entry point associated with device.
  void createOffloadTable(int32_t device_id, __tgt_offload_entry *begin,
                          __tgt_offload_entry *end) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    E.Table.EntriesBegin = begin;
    E.Table.EntriesEnd = end;
  }

  // Return true if the entry is associated with device.
  bool findOffloadEntry(int32_t device_id, void *addr) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    for (__tgt_offload_entry *i = E.Table.EntriesBegin, *e = E.Table.EntriesEnd;
         i < e; ++i) {
      if (i->addr == addr)
        return true;
    }

    return false;
  }

  // Return the pointer to the target entries table.
  __tgt_target_table *getOffloadEntriesTable(int32_t device_id) {
    assert(device_id < (int32_t)FuncGblEntries.size() &&
           "Unexpected device id!");
    FuncOrGblEntryTy &E = FuncGblEntries[device_id];

    return &E.Table;
  }

  RTLDeviceInfoTy(int32_t num_devices) { FuncGblEntries.resize(num_devices); }

  ~RTLDeviceInfoTy() {
    // Close dynamic libraries
    for (auto &lib : DynLibs) {
      if (lib.Handle) {
        dlclose(lib.Handle);
        remove(lib.FileName);
      }
    }
  }
};

static RTLDeviceInfoTy DeviceInfo(NUMBER_OF_DEVICES);
static AlveoGenericApp alveo_generic_app;

#ifdef __cplusplus
extern "C" {
#endif

int32_t __tgt_rtl_set_module(void* module) {

  std::string module_str((char*) module);

  DP("[alveo] module = %s\n", module_str.c_str());

  TIME_START
  if (0 != alveo_generic_app.program(module_str.c_str()))
      return OFFLOAD_FAIL;
  TIME_PRINT("program")

  DP("[alveo] __tgt_rtl_set_module success\n");

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_is_valid_binary(__tgt_device_image *image) {
  uint32_t is_valid_binary = elf_check_machine(image, EM_X86_64, 9010);

  DP("[alveo] __tgt_rtl_is_valid_binary\n");

  return is_valid_binary;
}

int32_t __tgt_rtl_number_of_devices() {
  return NUMBER_OF_DEVICES;
}

int32_t __tgt_rtl_init_device(int32_t device_id) {

  DP("[alveo] __tgt_rtl_init_device\n");
  alveo_generic_app.init();

  return OFFLOAD_SUCCESS;
}

__tgt_target_table *__tgt_rtl_load_binary(int32_t device_id,
                                          __tgt_device_image *image) {

  DP("Dev %d: load binary from " DPxMOD " image\n", device_id,
     DPxPTR(image->ImageStart));

  assert(device_id >= 0 && device_id < NUMBER_OF_DEVICES && "bad dev id");

  size_t ImageSize = (size_t)image->ImageEnd - (size_t)image->ImageStart;
  size_t NumEntries = (size_t)(image->EntriesEnd - image->EntriesBegin);
  DP("Expecting to have %zd entries defined.\n", NumEntries);

  // Is the library version incompatible with the header file?
  if (elf_version(EV_CURRENT) == EV_NONE) {
    DP("Incompatible ELF library!\n");
    return NULL;
  }

  // Obtain elf handler
  Elf *e = elf_memory((char *)image->ImageStart, ImageSize);
  if (!e) {
    DP("Unable to get ELF handle: %s!\n", elf_errmsg(-1));
    return NULL;
  }

  if (elf_kind(e) != ELF_K_ELF) {
    DP("Invalid Elf kind!\n");
    elf_end(e);
    return NULL;
  }

  // Find the entries section offset
  Elf_Scn *section = 0;
  Elf64_Off entries_offset = 0;

  size_t shstrndx;

  if (elf_getshdrstrndx(e, &shstrndx)) {
    DP("Unable to get ELF strings index!\n");
    elf_end(e);
    return NULL;
  }

  while ((section = elf_nextscn(e, section))) {
    GElf_Shdr hdr;
    gelf_getshdr(section, &hdr);

    if (!strcmp(elf_strptr(e, shstrndx, hdr.sh_name), OFFLOADSECTIONNAME)) {
      entries_offset = hdr.sh_addr;
      break;
    }
  }

  if (!entries_offset) {
    DP("Entries Section Offset Not Found\n");
    elf_end(e);
    return NULL;
  }

  DP("Offset of entries section is (" DPxMOD ").\n", DPxPTR(entries_offset));

  // load dynamic library and get the entry points. We use the dl library
  // to do the loading of the library, but we could do it directly to avoid the
  // dump to the temporary file.
  //
  // 1) Create tmp file with the library contents.
  // 2) Use dlopen to load the file and dlsym to retrieve the symbols.
  mkdir(".tmp", 0777);
  char tmp_name[] = ".tmp/tmpfile_XXXXXX";
  int tmp_fd = mkstemp(tmp_name);

  if (tmp_fd == -1) {
    elf_end(e);
    return NULL;
  }

  FILE *ftmp = fdopen(tmp_fd, "wb");

  if (!ftmp) {
    elf_end(e);
    return NULL;
  }

  fwrite(image->ImageStart, ImageSize, 1, ftmp);
  fclose(ftmp);

  DynLibTy Lib = {tmp_name, dlopen(tmp_name, RTLD_LAZY)};

  if (!Lib.Handle) {
    DP("Target library loading error: %s\n", dlerror());
    elf_end(e);
    return NULL;
  }

  DeviceInfo.DynLibs.push_back(Lib);

  struct link_map *libInfo = (struct link_map *)Lib.Handle;

  // The place where the entries info is loaded is the library base address
  // plus the offset determined from the ELF file.
  Elf64_Addr entries_addr = libInfo->l_addr + entries_offset;

  DP("Pointer to first entry to be loaded is (" DPxMOD ").\n",
      DPxPTR(entries_addr));

  // Table of pointers to all the entries in the target.
  __tgt_offload_entry *entries_table = (__tgt_offload_entry *)entries_addr;

  __tgt_offload_entry *entries_begin = &entries_table[0];
  __tgt_offload_entry *entries_end = entries_begin + NumEntries;

  if (!entries_begin) {
    DP("Can't obtain entries begin\n");
    elf_end(e);
    return NULL;
  }

  DP("Entries table range is (" DPxMOD ")->(" DPxMOD ")\n",
      DPxPTR(entries_begin), DPxPTR(entries_end));
  DeviceInfo.createOffloadTable(device_id, entries_begin, entries_end);

  elf_end(e);

  return DeviceInfo.getOffloadEntriesTable(device_id);
}

void *__tgt_rtl_data_alloc(int32_t device_id, int64_t size) {
  DP("[alveo] __tgt_rtl_data_alloc\n");

  TIME_START
  void *ptr = alveo_generic_app.alloc_buffer(size);
  TIME_PRINT("alloc")

  DP("[alveo] size = %d\n", size);

  if (ptr == NULL)
    DP("[alveo] ptr null\n");

  return ptr;
}

int32_t __tgt_rtl_data_submit(int32_t device_id, void *tgt_ptr, void *hst_ptr,
    int64_t size) {

  DP("[alveo] __tgt_rtl_data_submit\n");

  TIME_START

  alveo_generic_app.submit_buffer(tgt_ptr, hst_ptr, size);

  TIME_PRINT("submit")

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_retrieve(int32_t device_id, void *hst_ptr, void *tgt_ptr,
    int64_t size) {

  DP("[alveo] __tgt_rtl_data_retrieve\n");

  TIME_START
  alveo_generic_app.retrieve_buffer(    hst_ptr, tgt_ptr, size);
  TIME_PRINT("retrieve")

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_data_delete(int32_t device_id, void *tgt_ptr) {

  DP("[alveo] __tgt_rtl_delete\n");

  TIME_START
  // alveo_generic_app.delete_buffer(tgt_ptr);
  TIME_PRINT("delete")

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_team_region(int32_t device_id, void *tgt_entry_ptr,
    void **tgt_args, int32_t arg_num, int32_t team_num, int32_t thread_limit,
    uint64_t loop_tripcount) {

  DP("[alveo] __tgt_rtl_run_target_team_region\n");
  TIME_START
  alveo_generic_app.run();
  TIME_PRINT("run")

  return OFFLOAD_SUCCESS;
}

int32_t __tgt_rtl_run_target_region(int32_t device_id, void *tgt_entry_ptr,
    void **tgt_args, int32_t arg_num) {

  DP("[alveo] __tgt_rtl_run_target_region\n");

  // use one team and one thread.
  return __tgt_rtl_run_target_team_region(device_id, tgt_entry_ptr, tgt_args,
                                          arg_num, 1, 1, 0);
}

#ifdef __cplusplus
}
#endif
