cmake_minimum_required(VERSION 2.8.1)

add_executable(jios-test
    jout_test.cpp
    test.cpp
    assertion_failed.cpp
)
target_link_libraries(jios-test jios ${Boost_LIBRARIES})

add_test(NAME jios-test COMMAND jios-test -l message)
