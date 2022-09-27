proto_library(paddle_proto SRCS framework.proto)

core_gather_headers()

set(paddle_example_src main.cc)

add_executable(paddle_exmaple ${paddle_example_src})
target_link_libraries(paddle_exmaple paddle_proto ${third_party} protobuf)