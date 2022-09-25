set(GIT_URL "https://github.com")

set(THIRD_PARTY_PATH
    "${CMAKE_BINARY_DIR}/third_party"
    CACHE STRING
          "A path setting third party libraries download & build directories.")
set(THIRD_PARTY_CACHE_PATH
    "${CMAKE_SOURCE_DIR}"
    CACHE STRING
          "A path cache third party source code to avoid repeated download.")

set(THIRD_PARTY_BUILD_TYPE Release)
set(third_party_deps)

include(ProcessorCount)
ProcessorCount(NPROC)

include(external/gflags) # download, build, install gflags
include(external/glog)
include(external/protobuf) # find first, then download, build, install protobuf
list(APPEND third_party_deps extern_gflags extern_glog)
if(TARGET extern_protobuf)
  list(APPEND third_party_deps extern_protobuf)
endif()

if(WITH_MLIR)
  include(external/llvm)
  list(APPEND third_party_deps ${llvm_libs})
endif()

add_custom_target(third_party ALL DEPENDS ${third_party_deps})
