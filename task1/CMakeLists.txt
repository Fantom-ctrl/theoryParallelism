cmake_minimum_required(VERSION 3.5)
project(sine_array LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(USE_DOUBLE "Use double instead of float" OFF)

add_executable(sine_array doubleAndFloat.cpp)

if(USE_DOUBLE)
    target_compile_definitions(sine_array PRIVATE USE_DOUBLE=1)
else()
    target_compile_definitions(sine_array PRIVATE USE_DOUBLE=0)
endif()
