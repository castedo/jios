cmake_minimum_required(VERSION 2.8.1)

project(jios)

# Uncomment for debugging build:
# set(CMAKE_VERBOSE_MAKEFILE ON)
# add_definitions(-H)

add_definitions(-std=c++11)

include_directories(.)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  add_definitions(-DBOOST_ENABLE_ASSERT_HANDLER)
endif()

enable_testing()

### boost

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.56.0 REQUIRED COMPONENTS
               system filesystem unit_test_framework)
include_directories(${Boost_INCLUDE_DIRS})

### google protocol buffers

find_package(Protobuf REQUIRED)

include_directories(${PROTOBUF_INCLUDE_DIRS})

# precompiled headers and google protobuf generated headers files:
include_directories(${CMAKE_BINARY_DIR})

### project parts

add_subdirectory(lib)
add_subdirectory(test)

