cmake_minimum_required(VERSION 2.8.1)

add_executable(jios-test
    jin_test.cpp
    jout_test.cpp
    express_test.cpp
    parser_test.cpp
    test.cpp
    assertion_failed.cpp
)
target_link_libraries(jios-test jios ${Boost_LIBRARIES})

add_test(NAME jios-test COMMAND jios-test -l message)
