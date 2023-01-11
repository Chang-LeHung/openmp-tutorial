cmake_minimum_required(VERSION 3.16.3)
project(openmp_tutorial)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_COMPILER "/usr/bin/gcc")

set(CMAKE_C_FLAGS "-fopenmp ${CMAKE_C_FLAGS}")



add_executable(active_level src/runtime/active_level.c)
