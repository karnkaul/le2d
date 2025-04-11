cmake_minimum_required(VERSION 3.23)

set(project_name le2d)
project(${project_name} VERSION "0.1.0")

option(LE2D_BUILD_ASSED "Build le2d Asset Editor" ${PROJECT_IS_TOP_LEVEL})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

add_subdirectory(ext)

add_subdirectory(lib)

if(LE2D_BUILD_ASSED)
  add_subdirectory(assed)
endif()
