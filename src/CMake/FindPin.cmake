# - Find PIN SDK.
# This module finds a pin installation and selects a default one.
#
# Author: Manuel Niekamp
# Email: niekma@upb.de
#
# The following variables are set after the configuration is done:
#
#  PIN_FOUND            - Set to TRUE if pin was found.
#  PIN_ROOT_DIR         - base directory of pin
#  PIN_INCLUDE_DIRS     - Include directories.
#  PIN_LIBRARY_DIRS     - compile time link dirs, useful for
#                         rpath on UNIX. Typically an empty string
#                         in WIN32 environment.
#  PIN_DEFINITIONS      - Contains defines required to compile/link
#                         against pin
#  PIN_COMPILE_FLAGS    - Compiler flags for C an C++
#  PIN_CXX_FLAGS        - Extra C++ compiler flags
#  PIN_C_FLAGS          - Extra C compiler flags
#  PIN_USE_FILE         - Convenience include file.
#  PIN_CPU_ARCH         - ia32, ia64

set(PIN_FOUND false)

# Add the convenience use file if available.
set(PIN_USE_FILE "")
get_filename_component(TMP_CURRENT_LIST_DIR ${CMAKE_CURRENT_LIST_FILE} PATH)
# Prefer an existing customized version
if(EXISTS "${TMP_CURRENT_LIST_DIR}/UsePin.cmake")
  set(PIN_USE_FILE "${TMP_CURRENT_LIST_DIR}/UsePin.cmake")
else(EXISTS "${TMP_CURRENT_LIST_DIR}/UsePin.cmake")
  set(PIN_USE_FILE UsePin.cmake)
endif(EXISTS "${TMP_CURRENT_LIST_DIR}/UsePin.cmake")

execute_process(
  COMMAND uname -m
  OUTPUT_VARIABLE PIN_CPU_ARCH
  RESULT_VARIABLE uname_result
  OUTPUT_STRIP_TRAILING_WHITESPACE
  ERROR_QUIET
)

# enable the following for 32bit compilation
set(PIN_CPU_ARCH "x86")

if("${PIN_CPU_ARCH}" STREQUAL "x86_64")
  set(PIN_CPU_ARCH "ia32e")
  set(PIN_CPU_ARCH_LONG "intel64")
elseif("${PIN_CPU_ARCH}" STREQUAL "amd64")
  set(PIN_CPU_ARCH "ia32e")
  set(PIN_CPU_ARCH_LONG "intel64")
elseif("${PIN_CPU_ARCH}" STREQUAL "i686")
  set(PIN_CPU_ARCH "ia32")
  set(PIN_CPU_ARCH_LONG "ia32")
elseif("${PIN_CPU_ARCH}" STREQUAL "x86")
  set(PIN_CPU_ARCH "ia32")
  set(PIN_CPU_ARCH_LONG "ia32")
elseif("${PIN_CPU_ARCH}" STREQUAL "i386")
  set(PIN_CPU_ARCH "ia32")
  set(PIN_CPU_ARCH_LONG "ia32")
elseif("${PIN_CPU_ARCH}" STREQUAL "ia64")
  set(PIN_CPU_ARCH "ipf")
  set(PIN_CPU_ARCH_LONG "ia62")
endif("${PIN_CPU_ARCH}" STREQUAL "x86_64")

message(STATUS "PIN_CPU_ARCH: ${PIN_CPU_ARCH}")

find_path(PIN_ROOT_DIR
  NAMES source/include/pin/pin.H
  PATHS $ENV{PIN_ROOT_DIR}
  DOC "Pin's base directory"
)

if(NOT PIN_ROOT_DIR)
  message(FATAL_ERROR
    "\nPin not found!\n"
    "Please set the environment variable PIN_ROOT_DIR to the base directory"
    " of the pin library.\n"
  )
endif(NOT PIN_ROOT_DIR)

message(STATUS "PIN_ROOT_DIR: ${PIN_ROOT_DIR}")

set(PIN_INCLUDE_DIRS
  ${PIN_ROOT_DIR}/extras/xed2-${PIN_CPU_ARCH_LONG}/include
#  ${PIN_ROOT_DIR}/extras/xed-${PIN_CPU_ARCH_LONG}/include
  ${PIN_ROOT_DIR}/source/include/pin
  ${PIN_ROOT_DIR}/source/include/pin/gen
  ${PIN_ROOT_DIR}/extras/components/include
)

set(PIN_LIBRARY_DIRS
  ${PIN_ROOT_DIR}/extras/xed2-${PIN_CPU_ARCH_LONG}/lib
#  ${PIN_ROOT_DIR}/extras/xed-${PIN_CPU_ARCH_LONG}/lib
  ${PIN_ROOT_DIR}/${PIN_CPU_ARCH_LONG}/lib 
  ${PIN_ROOT_DIR}/${PIN_CPU_ARCH_LONG}/lib-ext
)

set(PIN_VERSION_SCRIPT ${PIN_ROOT_DIR}/source/include/pin/pintool.ver)

#set(PIN_COMPILE_FLAGS "-Wall -Werror -Wno-unknown-pragmas -O3 -fomit-frame-pointer -fno-strict-aliasing -DBOOST_LOG_DYN_LINK")
set(PIN_COMPILE_FLAGS "-Wall -Wno-unknown-pragmas -O3 -fomit-frame-pointer -fno-strict-aliasing -DBOOST_LOG_DYN_LINK")
set(PIN_C_FLAGS "${PIN_COMPILE_FLAGS}")
# enable the following  for 32 bit compilation
set(PIN_CXX_FLAGS "${PIN_COMPILE_FLAGS} -MMD -m32")
#set(PIN_CXX_FLAGS "${PIN_COMPILE_FLAGS} -MMD")
#set(PIN_LINKER_FLAGS "-Wl,--hash-style=sysv -shared -Wl,-Bsymbolic -Wl,--version-script=${PIN_VERSION_SCRIPT}")
set(PIN_LINKER_FLAGS "-Wl,--hash-style=sysv -Wl,-Bsymbolic -Wl,--version-script=${PIN_VERSION_SCRIPT}")

set(PIN_DEFINITIONS "")
list(APPEND PIN_DEFINITIONS TARGET_LINUX BIGARRAY_MULTIPLIER=1 USING_XED)

if("${PIN_CPU_ARCH}" STREQUAL "ia32e")
  list(APPEND PIN_DEFINITIONS TARGET_IA32E HOST_IA32E)
elseif("${PIN_CPU_ARCH}" STREQUAL "ia32")
  list(APPEND PIN_DEFINITIONS TARGET_IA32 HOST_IA32)
elseif("${PIN_CPU_ARCH}" STREQUAL "ipf")
  list(APPEND PIN_DEFINITIONS TARGET_IPF HOST_IPF)
endif("${PIN_CPU_ARCH}" STREQUAL "ia32e")

set(PIN_FOUND true)

