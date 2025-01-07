cmake_minimum_required(VERSION 3.23)

project(le2d VERSION "0.1.0")

option(LE2D_BUILD_EXAMPLE "Build le2d-example" ${PROJECT_IS_TOP_LEVEL})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

add_subdirectory(ext)

add_subdirectory(lib)

if(LE2D_BUILD_EXAMPLE)
  add_subdirectory(example)
endif()
