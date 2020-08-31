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
