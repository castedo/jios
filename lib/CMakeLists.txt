cmake_minimum_required(VERSION 2.8.1)

add_library(jios
    json_oj.cpp
    jin.cpp
    protobuf_oj.cpp
)

target_link_libraries(jios
    ${Boost_LIBRARIES}
)

