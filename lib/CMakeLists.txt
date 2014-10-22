cmake_minimum_required(VERSION 2.8.1)

add_library(jios
    jin.cpp
    json_out.cpp
    json_in.cpp
    protobuf_oj.cpp
    protobuf_ij.cpp
)

target_link_libraries(jios
    json-c
    ${Boost_LIBRARIES}
)

