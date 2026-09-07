cmake_minimum_required(VERSION 3.14)
project(ComputationalGeometry CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimization flags
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()
add_compile_options(-O2 -Wall -Wextra)

include_directories(src)

add_executable(part_A src/main_A.cpp)
add_executable(part_B src/main_B.cpp)
add_executable(part_C src/main_C.cpp)
add_executable(part_D src/main_D.cpp)
