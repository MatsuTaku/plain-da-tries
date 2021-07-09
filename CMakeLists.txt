cmake_minimum_required(VERSION 3.10)

project(plain-da-tries CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "-Wall -Wno-sign-compare -march=native -O3")

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

add_subdirectory(libbo EXCLUDE_FROM_ALL)

add_executable(bench benchmark.cpp)
target_link_libraries(bench libbo)

enable_testing()
file(GLOB TEST_SOURCES *_test.cpp)
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_SOURCE_NAME ${TEST_SOURCE} NAME_WE)

    add_executable(${TEST_SOURCE_NAME} ${TEST_SOURCE})
    target_link_libraries(${TEST_SOURCE_NAME} libbo)
    add_test(NAME ${TEST_SOURCE_NAME} COMMAND ${TEST_SOURCE_NAME})
endforeach()
