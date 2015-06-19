set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/CMake/")

project(PinICap)
cmake_minimum_required(VERSION 2.8)

set(CMAKE_BUILD_TYPE "Debug")

include(FindPin)
include(UsePin)

list(APPEND CMAKE_CXX_FLAGS "-std=c++11 ${CMAKE_CXX_FLAGS}")

#include_directories("/home/bigraph/.local/include")
if(BOOST_INCLUDE_DIR)
  include_directories(AFTER ${BOOST_INCLUDE_DIR})
endif(BOOST_INCLUDE_DIR)

ADD_PINTOOL(vtrace
  src/main.cpp
  src/lib/cap/tracer.cpp
  src/lib/cap/parser.cpp
#  src/lib/cap/saver.cpp
  src/lib/cap/graph.cpp
#  src/lib/cap/trace_graph.cpp
  src/lib/type/trace.pb.cc
  src/lib/type/instruction.cpp)

#ADD_PINTOOL(print_value
#  src/print_value.cpp
#  src/lib/cap/tracer.cpp
#  src/lib/cap/parser.cpp
#  src/lib/type/trace.pb.cc
#  src/lib/type/instruction.cpp)
