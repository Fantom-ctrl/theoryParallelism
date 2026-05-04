cmake_minimum_required(VERSION 3.10)
project(serv LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenMP REQUIRED)

add_executable(main main.cpp)
add_executable(tests tests.cpp)

target_include_directories(main PRIVATE argparse/include)

target_link_libraries(main PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(main PRIVATE -O3)