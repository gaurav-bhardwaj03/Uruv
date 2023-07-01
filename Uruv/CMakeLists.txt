cmake_minimum_required(VERSION 3.10)
project(Uruv)

set(CMAKE_CXX_STANDARD 17)

add_executable(Uruv
        Benchmark.cpp
#        Hotkeys_expt.cpp
#        Expt1.cpp
        )

target_link_libraries(
        Uruv
        -lpthread
        -fopenmp
)
