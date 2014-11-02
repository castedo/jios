cmake_minimum_required(VERSION 2.8.1)

add_library(jios
    jout.cpp
    jin.cpp
    json_out.cpp
    json_in.cpp
    protobuf_oj.cpp
    protobuf_ij.cpp
    jsonc_parser.cpp
)

target_link_libraries(jios
    json-c
    ${Boost_LIBRARIES}
)

