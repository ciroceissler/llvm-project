//===-- elf_common.c - Common ELF functionality -------------------*- C -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is dual licensed under the MIT and the University of Illinois Open
// Source Licenses. See LICENSE.txt for details.
//
//===----------------------------------------------------------------------===//
//
// Common ELF functionality for target plugins.
// Must be included in the plugin source file AFTER omptarget.h has been
// included and macro DP(...) has been defined.
// .
//
//===----------------------------------------------------------------------===//

#if !(defined(_OMPTARGET_H_) && defined(DP))
#error Include elf_common.c in the plugin source AFTER omptarget.h has been\
 included and macro DP(...) has been defined.
#endif

#include <elf.h>
#include <libelf.h>
#include <string.h>

static inline int32_t map_tgt_configuration(Elf_Scn* section,
    GElf_Shdr shdr,
   const char *img_begin,
   __tgt_configuration** cfg) {

  size_t n = 0;
  Elf_Data *data = NULL;

  while ( (n < shdr.sh_size) &&
          (data = elf_getdata(section, data)) != NULL) {
    *cfg = (__tgt_configuration*) data->d_buf;
  }

  return 1;
}

static inline int32_t find_section_shdr(Elf_Scn* section,
    GElf_Shdr &shdr) {

  if (gelf_getshdr(section, &shdr) != &shdr) {
    DP("Could not get section data\n");

    return 0;
  }

  if (shdr.sh_type == SHT_NOBITS)
    return -1;

  return 1;
}

static inline int32_t find_section_by_name(Elf* e,
    Elf_Scn** section,
    const char* section_name) {

  *section = 0;

  size_t shstrndx;

  if (elf_getshdrstrndx(e, &shstrndx)) {
    DP("Unable to get ELF strings index!\n");

    return 0;
  }

  while ((*section = elf_nextscn(e, *section))) {
    GElf_Shdr hdr;
    gelf_getshdr(*section, &hdr);

    if (!strcmp(elf_strptr(e, shstrndx, hdr.sh_name), section_name)) {
      DP("found section name: %s\n", section_name);

      return 1;
    }
  }

  return 0;
}

// one architecture supports multiple targets, e.g.,
// x86_64 supports others devices like cloud, smartnic, ...
static inline int32_t check_devices(Elf *e,
    char* img_begin,
    uint16_t MachineID,
    uint16_t target_id,
    uint16_t sub_target_id) {

  int32_t sub_target_check = 1;

  if (EM_X86_64 == target_id) {
    Elf_Scn* section = 0;
    GElf_Shdr shdr;
    __tgt_configuration* cfg;

   int has_failed = 0;

    if (!find_section_by_name(e, &section, ".omp_offloading.configuration"))
      has_failed = 1;
    if (has_failed || !find_section_shdr(section, shdr))
      has_failed = 2;
    if (has_failed || !map_tgt_configuration(section, shdr, img_begin, &cfg))
      has_failed = 4;

    if (0 == has_failed) {
      DP("sub_target_id = %d | cfg->sub_target_id = %d\n",
          sub_target_id,
          cfg->sub_target_id);
      DP("module = %s\n", cfg->module);
      sub_target_check = (sub_target_id == cfg->sub_target_id);
    } else {
      sub_target_check = 1;
    }
  }

  return (MachineID == target_id) & sub_target_check;
}

// Check whether an image is valid for execution on target_id

// Check whether an image is valid for execution on target_id and sub_target_id
static inline int32_t elf_check_machine(__tgt_device_image *image,
    uint16_t target_id,
    uint16_t sub_target_id) {

  // Is the library version incompatible with the header file?
  if (elf_version(EV_CURRENT) == EV_NONE) {
    DP("Incompatible ELF library!\n");
    return 0;
  }

  char *img_begin = (char *)image->ImageStart;
  char *img_end = (char *)image->ImageEnd;
  size_t img_size = img_end - img_begin;

  // Obtain elf handler
  Elf *e = elf_memory(img_begin, img_size);
  if (!e) {
    DP("Unable to get ELF handle: %s!\n", elf_errmsg(-1));
    return 0;
  }

  // Check if ELF is the right kind.
  if (elf_kind(e) != ELF_K_ELF) {
    DP("Unexpected ELF type!\n");
    return 0;
  }
  Elf64_Ehdr *eh64 = elf64_getehdr(e);
  Elf32_Ehdr *eh32 = elf32_getehdr(e);

  if (!eh64 && !eh32) {
    DP("Unable to get machine ID from ELF file!\n");
    elf_end(e);
    return 0;
  }

  uint16_t MachineID;
  if (eh64 && !eh32)
    MachineID = eh64->e_machine;
  else if (eh32 && !eh64)
    MachineID = eh32->e_machine;
  else {
    DP("Ambiguous ELF header!\n");
    elf_end(e);
    return 0;
  }

  int32_t check;
  check = check_devices(e, img_begin, MachineID, target_id, sub_target_id);

  elf_end(e);

  return check;
}

static inline int32_t get_tgt_configuration_module(__tgt_device_image *image,
    __tgt_configuration **cfg) {

  int has_failed = 0;
  Elf_Scn* section = 0;
  GElf_Shdr shdr;

  char *img_begin = (char *)image->ImageStart;
  char *img_end = (char *)image->ImageEnd;
  size_t img_size = img_end - img_begin;

  // Obtain elf handler
  Elf *e = elf_memory(img_begin, img_size);

  if (!find_section_by_name(e, &section, ".omp_offloading.configuration"))
    has_failed = 1;
  if (has_failed || !find_section_shdr(section, shdr))
    has_failed = 2;
  if (has_failed || !map_tgt_configuration(section, shdr, img_begin, cfg))
    has_failed = 3;

  elf_end(e);

  return has_failed;
}


// Check whether an image is valid for execution on target_id
static inline int32_t elf_check_machine(__tgt_device_image *image,
    uint16_t target_id) {

  // Is the library version incompatible with the header file?
  if (elf_version(EV_CURRENT) == EV_NONE) {
    DP("Incompatible ELF library!\n");
    return 0;
  }

  char *img_begin = (char *)image->ImageStart;
  char *img_end = (char *)image->ImageEnd;
  size_t img_size = img_end - img_begin;

  // Obtain elf handler
  Elf *e = elf_memory(img_begin, img_size);
  if (!e) {
    DP("Unable to get ELF handle: %s!\n", elf_errmsg(-1));
    return 0;
  }

  // Check if ELF is the right kind.
  if (elf_kind(e) != ELF_K_ELF) {
    DP("Unexpected ELF type!\n");
    return 0;
  }
  Elf64_Ehdr *eh64 = elf64_getehdr(e);
  Elf32_Ehdr *eh32 = elf32_getehdr(e);

  if (!eh64 && !eh32) {
    DP("Unable to get machine ID from ELF file!\n");
    elf_end(e);
    return 0;
  }

  uint16_t MachineID;
  if (eh64 && !eh32)
    MachineID = eh64->e_machine;
  else if (eh32 && !eh64)
    MachineID = eh32->e_machine;
  else {
    DP("Ambiguous ELF header!\n");
    elf_end(e);
    return 0;
  }

  elf_end(e);
  return MachineID == target_id;
}
