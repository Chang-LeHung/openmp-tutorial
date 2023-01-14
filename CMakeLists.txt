cmake_minimum_required(VERSION 3.16.3)
project(openmp_tutorial)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_BUILD_TYPE "Debug")

set(CMAKE_C_FLAGS "-fopenmp -g ${CMAKE_C_FLAGS}")



add_executable(active_level src/runtime/active_level.c)

add_executable(thread_limit src/runtime/thread_limit.c)
add_executable(section src/runtime/section.c)

add_executable(example01 src/sections/example01.c)
add_executable(critical src/sync/critical.c)
add_executable(for src/directives/for.c)
add_executable(single src/directives/single.c)
add_executable(order src/directives/order.c)
add_executable(flush src/directives/flush.c)
add_executable(omp_get_wtime src/runtime/omp_get_wtime.c)
add_executable(omp_in_parallel src/runtime/omp_in_parallel.c)