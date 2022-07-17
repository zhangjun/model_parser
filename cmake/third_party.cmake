set(GIT_URL "https://github.com")
include(external/gflags) # download, build, install gflags
include(external/glog)
include(external/protobuf) # find first, then download, build, install protobuf
if(TARGET extern_protobuf)
  list(APPEND third_party_deps extern_protobuf)
endif()
