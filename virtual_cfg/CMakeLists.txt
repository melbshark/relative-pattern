project(virtual_cfg)
cmake_minimum_required(VERSION 2.8)
aux_source_directory(. SRC_LIST)

if(NOT DEFINED BOOST_ROOT_DIR)
  message(FATAL_ERROR "\nBoost not found!\n"
    "Please set the environment variable BOOST_ROOT_DIR to the base directory"
    " of the Boost library.\n")
endif(NOT DEFINED BOOST_ROOT_DIR)
message(STATUS "BOOST_ROOT_DIR: ${BOOST_ROOT_DIR}")

set(INCLUDE_DIRS
  ${BOOST_ROOT_DIR}
)

include_directories(${INCLUDE_DIRS})

add_executable(${PROJECT_NAME} ${SRC_LIST})

