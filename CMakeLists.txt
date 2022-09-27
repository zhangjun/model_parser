cmake_minimum_required(VERSION 3.10)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

include(system)
project(model_parser CXX C)
set(CMAKE_CUDA_STANDARD 14)
set(CMAKE_BUILD_TYPE Release)

find_package(Threads REQUIRED)

option(WITH_MLIR "Compile model_parser with MLIR" OFF)

set(MODEL_SOURCE_DIR "${CMAKE_SOURCE_DIR}/model_parser")
set(MODEL_BINARY_DIR "${CMAKE_BINARY_DIR}/model_parser")

set(paddle_src CACHE INTERNAL "" FORCE)
include(configure)
include(ccache)
include(core)
include(third_party)
include(flags)

# compile flags
set(COMMON_FLAGS -fPIC)
foreach(flag ${COMMON_FLAGS})
  safe_set_cflag(CMAKE_C_FLAGS ${flag})
  safe_set_cxxflag(CMAKE_CXX_FLAGS ${flag})
endforeach()

include_directories("${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(common)
add_subdirectory(support)
add_subdirectory(paddle)
add_subdirectory(examples)

cc_library(
  model_parser SHARED
  SRCS ${paddle_src}
  DEPS paddle_framework_proto)

cc_library(
  model_parser_static
  SRCS ${paddle_src}
  DEPS paddle_framework_proto)
