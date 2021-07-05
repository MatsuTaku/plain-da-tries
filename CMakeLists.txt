cmake_minimum_required(VERSION 3.10)

project(plain-da-tries CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -Wno-sign-compare -march=native -O3")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_executable(bench benchmark.cpp)

add_subdirectory(libbo EXCLUDE_FROM_ALL)
target_link_libraries(bench PRIVATE libbo)

enable_testing()

add_executable(index_sum_convolution_test index_sum_convolution_test.cpp)
target_link_libraries(index_sum_convolution_test libbo)
add_test(NAME index_sum_convolution_test COMMAND index_sum_convolution_test)

add_executable(index_xor_convolution_test index_xor_convolution_test.cpp)
target_link_libraries(index_xor_convolution_test libbo)
add_test(NAME index_xor_convolution_test COMMAND index_xor_convolution_test)
