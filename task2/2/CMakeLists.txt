cmake_minimum_required(VERSION 3.10)
project(2part LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(main main.cpp)

target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(main PRIVATE -O3)
