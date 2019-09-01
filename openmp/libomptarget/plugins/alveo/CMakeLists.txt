##===----------------------------------------------------------------------===##
#
#                     The LLVM Compiler Infrastructure
#
# This file is dual licensed under the MIT and the University of Illinois Open
# Source Licenses. See LICENSE.txt for details.
#
##===----------------------------------------------------------------------===##
#
# Build a plugin for a Xilinx Alveo U200/U250 machine if available.
#
##===----------------------------------------------------------------------===##
if(LIBOMPTARGET_DEP_LIBELF_FOUND)
if(LIBOMPTARGET_DEP_LIBFFI_FOUND)
# if(LIBOMPTARGET_DEP_XILINX_VIVADO_FOUND)
# if(LIBOMPTARGET_DEP_XILINX_SDX_FOUND)
  libomptarget_say("Building Xilinx Alveo U200/U250 offloading plugin.")

  # Define the suffix for the runtime messaging dumps.
  add_definitions(-DTARGET_NAME=ALVEO)

  # Xilinx

  message(STATUS "the before value of SDX_PLATFORM: " $ENV{SDX_PLATFORM})
  if (NOT DEFINED ENV{SDX_PLATFORM})
    set(ENV{SDX_PLATFORM} "xilinx_aws-vu9p-f1-04261818_dynamic_5_0")
  endif()
  message(STATUS "the after value of SDX_PLATFORM: " $ENV{SDX_PLATFORM})

  add_definitions(-DSDX_PLATFORM=$ENV{SDX_PLATFORM})
  add_definitions(-D__USE_XOPEN2K8)

  if(LIBOMPTARGET_CMAKE_BUILD_TYPE MATCHES debug)
    add_definitions(-DAWSF1_ERROR_REPORT)
  endif()

  message(STATUS "the value of SDX_PLATFORM: " $ENV{SDX_PLATFORM})

  include_directories(${LIBOMPTARGET_DEP_LIBELF_INCLUDE_DIRS})
  include_directories(${LIBOMPTARGET_DEP_LIBFFI_INCLUDE_DIRS})
  # include_directories(${LIBOMPTARGET_DEP_XILINX_VIVADO_INCLUDE_DIRS})
  # include_directories(${LIBOMPTARGET_DEP_XILINX_SDX_INCLUDE_DIRS})

  add_library(omptarget.rtl.alveo SHARED
              src/util/alveo_generic_app.cpp
              src/util/utils.cpp
              src/rtl.cpp)

  # Install plugin under the lib destination folder.
  if(CMAKE_SYSTEM_NAME MATCHES "Windows")
    install(TARGETS omptarget.rtl.alveo RUNTIME DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
  else()
    install(TARGETS omptarget.rtl.alveo LIBRARY DESTINATION lib${LIBOMPTARGET_LIBDIR_SUFFIX})
  endif()

  target_link_libraries(omptarget.rtl.alveo
    ${LIBOMPTARGET_DEP_LIBFFI_LIBRARIES}
    ${LIBOMPTARGET_DEP_LIBELF_LIBRARIES}
    # ${LIBOMPTARGET_DEP_XILINX_SDX_LIBRARIES}
    pthread
    rt
    dl
    "-fmessage-length=0 -Wl,--version-script=${CMAKE_CURRENT_SOURCE_DIR}/../exports")

  # Report to the parent scope that we are building a plugin for Xilinx Alveo U200/U250.
  set(LIBOMPTARGET_SYSTEM_TARGETS "${LIBOMPTARGET_SYSTEM_TARGETS} x86_64" PARENT_SCOPE)
# else(LIBOMPTARGET_DEP_XILINX_SDX_FOUND)
#   libomptarget_say("Not building Xilinx Alveo U200/U250 offloading plugin: Xilinx SDx tools not found.")
# endif(LIBOMPTARGET_DEP_XILINX_SDX_FOUND)
# else(LIBOMPTARGET_DEP_XILINX_VIVADO_FOUND)
#   libomptarget_say("Not building Xilinx Alveo U200/U250 offloading plugin: Xilinx Vivado tools not found.")
# endif(LIBOMPTARGET_DEP_XILINX_VIVADO_FOUND)
else(LIBOMPTARGET_DEP_LIBFFI_FOUND)
  libomptarget_say("Not building Xilinx Alveo U200/U250 offloading plugin: libffi dependency not found.")
endif(LIBOMPTARGET_DEP_LIBFFI_FOUND)
else(LIBOMPTARGET_DEP_LIBELF_FOUND)
  libomptarget_say("Not building Xilinx Alveo U200/U250 offloading plugin: libelf dependency not found.")
endif(LIBOMPTARGET_DEP_LIBELF_FOUND)

