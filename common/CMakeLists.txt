core_gather_headers()
set(core_includes
    "${core_includes};common/dtype.def"
    CACHE INTERNAL "")

gather_srcs(
  paddle_src
  SRCS
  dtype.cc
  global.cc
  target.cc
  type.cc
  shared.cc
  object.cc
  string.cc
  buffer.cc
  memory.cc)
