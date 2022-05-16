cmake_minimum_required(VERSION 3.10)
project(DatasetGen)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./build)
set(CMAKE_BUILD_TYPE Debug)

add_executable(DatasetGen dataset_generator.cpp)

target_link_libraries(DatasetGen
        -lpthread
        -latomic
)
