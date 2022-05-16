cmake_minimum_required(VERSION 3.10)
project(Uruv)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./build)
set(CMAKE_BUILD_TYPE Debug)

add_executable(Uruv
        Benchmark.cpp
        )

target_link_libraries(
        Uruv
        -lpthread
        -ldl
        -latomic
)