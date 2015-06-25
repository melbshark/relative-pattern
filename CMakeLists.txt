project(PinICap)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE "None")

if(NOT PIN_ROOT_DIR)
  message(FATAL_ERROR
    "\nPin not found!\n"
    "Please set the environment variable PIN_ROOT_DIR to the base directory"
    " of the pin library.\n"
  )
endif(NOT PIN_ROOT_DIR)
message(STATUS "PIN_ROOT_DIR: ${PIN_ROOT_DIR}")

set(PIN_CPU_ARCH "ia32")
set(PIN_CPU_ARCH_LONG "ia32")

set(PIN_INCLUDE_DIRS
  ${PIN_ROOT_DIR}/extras/xed2-${PIN_CPU_ARCH_LONG}/include
  ${PIN_ROOT_DIR}/source/include/pin
  ${PIN_ROOT_DIR}/source/include/pin/gen
  ${PIN_ROOT_DIR}/extras/components/include
  ${LOCAL_INCLUDE_DIR})

set(PIN_LIBRARY_DIRS
  ${PIN_ROOT_DIR}/extras/xed2-${PIN_CPU_ARCH_LONG}/lib
  ${PIN_ROOT_DIR}/${PIN_CPU_ARCH_LONG}/lib
  ${PIN_ROOT_DIR}/${PIN_CPU_ARCH_LONG}/lib-ext)

set(SOURCES
  src/main.cpp
  src/lib/cap/tracer.cpp
  src/lib/cap/parser.cpp
  src/lib/cap/graph.cpp
  src/lib/type/instruction.cpp)

# ========================================= start host configuration

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
  set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")

  set(PIN_DEFS "/D TARGET_IA32 /D HOST_IA32 /D TARGET_WINDOWS /D USING_XED /D_SECURE_SCL=0 /D_ITERATOR_DEBUG_LEVEL=0 /Zc:auto")
  set(LINKER_DEFS "/D _WINDLL /D _MBCS /GS- /MT /EHsc /Ox /O2 /Ob2 /nologo")

  set(PIN_LIBRARY_DIRS
    ${PIN_ROOT_DIR}/extras/xed-${PIN_CPU_ARCH_LONG}/lib
    ${PIN_ROOT_DIR}/${PIN_CPU_ARCH_LONG}/lib
    ${PIN_ROOT_DIR}/${PIN_CPU_ARCH_LONG}/lib-ext)

  set(PIN_LIBS pin pinvm libxed ntdll-32 dbghelp)

  set(OTHER_LIBS libcpmt libcmt)

  if(NOT DEFINED BOOST_ROOT_DIR)
    message(FATAL_ERROR "\nBoost not found!\n"
      "Please set the environment variable BOOST_ROOT_DIR to the base directory"
      " of the Boost library.\n")
  endif(NOT DEFINED BOOST_ROOT_DIR)
  message(STATUS "BOOST_ROOT_DIR: ${BOOST_ROOT_DIR}")

  set(INCLUDE_DIRS
    ${PIN_INCLUDE_DIRS}
    ${BOOST_ROOT_DIR})

  include_directories(${INCLUDE_DIRS})
  link_directories(${PIN_LIBRARY_DIRS})

  add_definitions(${PIN_DEFS})
  add_definitions(${LINKER_DEFS})
  add_definitions(${FUNC_DEFS})

  set(CMAKE_CXX_FLAGS "${FUNC_DEFS} ${OTHER_DEFS} ${PIN_DEFS}")
  set(CMAKE_SHARED_LINKER_FLAGS "/DLL /EXPORT:main /NODEFAULTLIB /MACHINE:x86 /ENTRY:Ptrace_DllMainCRTStartup@12 /BASE:0x55000000")

  add_library(vtrace SHARED ${SOURCES})
  target_link_libraries(vtrace ${PIN_LIBS} ${OTHER_LIBS})
  
else()

  set(PIN_DEFINITIONS "")
  list(APPEND PIN_DEFINITIONS
    TARGET_LINUX
    BIGARRAY_MULTIPLIER=1
    USING_XED
    TARGET_IA32 HOST_IA32)

  set(PIN_VERSION_SCRIPT ${PIN_ROOT_DIR}/source/include/pin/pintool.ver)

  set(PIN_COMPILE_FLAGS "-Wall -Wno-unknown-pragmas -std=c++11 -m32 -MMD -O3 -fomit-frame-pointer -fno-strict-aliasing -fno-stack-protector")

  set(PIN_LINKER_FLAGS "-Wl,--hash-style=sysv -Wl,-Bsymbolic -Wl,--version-script=${PIN_VERSION_SCRIPT}")

  set(PIN_LINK_LIBS pin xed dwarf elf dl protobuf)

  link_directories(${PIN_LIBRARY_DIRS})
  include_directories(${PIN_INCLUDE_DIRS})

  # add Pintool
  add_library(vtrace SHARED ${SOURCES})

  # set compile and link flags
  set_source_files_properties(${SOURCES} PROPERTIES
    COMPILE_FLAGS "${PIN_COMPILE_FLAGS}"
    COMPILE_DEFINITIONS "${PIN_DEFINITIONS}")

  set_target_properties(vtrace PROPERTIES
    PREFIX ""
    SUFFIX ".pin"
    COMPILE_DEFINITIONS "${PIN_DEFINITIONS}"
    COMPILE_FLAGS "${PIN_COMPILE_FLAGS}"
    LINK_FLAGS "${PIN_LINKER_FLAGS} -v -m32")

  target_link_libraries(vtrace ${PIN_LINK_LIBS})
endif()

