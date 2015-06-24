project(jump_table)
cmake_minimum_required(VERSION 2.9)

aux_source_directory(. SRC_LIST)

include_directories(.)

add_definitions(-std=c++11)

add_executable(${PROJECT_NAME} ${SRC_LIST})
#target_compile_features(${PROJECT_NAME} PRIVATE cxx_autotype)

