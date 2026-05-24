cmake_minimum_required(VERSION 3.10)

project(seq CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_COMPILER pgc++)

set(CMAKE_CXX_FLAGS "-O2 -acc -Minfo=all")

find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(app jacobi.cpp laplace2d.cpp)

target_link_libraries(app Boost::program_options)