cmake_minimum_required(VERSION 3.24)

include(version.cmake)

set(project_name le2d)
project(${project_name} VERSION ${le2d_version})

option(LE2D_BUILD_ASSED "Build le2d Asset Editor" ${PROJECT_IS_TOP_LEVEL})
option(LE2D_BUILD_SPIRV2CPP "Build spirv2cpp" ${PROJECT_IS_TOP_LEVEL})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_DEBUG_POSTFIX "-d")

add_subdirectory(ext)

add_subdirectory(lib)

if(LE2D_BUILD_ASSED)
  add_subdirectory(assed)
endif()

if(LE2D_BUILD_SPIRV2CPP)
  add_subdirectory(spirv2cpp)
endif()
