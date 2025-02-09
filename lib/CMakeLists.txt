add_library(${PROJECT_NAME})
add_library(${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME} PUBLIC
  kvf::kvf
  djson::djson
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  klib::klib-warnings
  klib::klib-ndwerror
  klib::klib-msbuild-mp
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src
)

file(GLOB_RECURSE headers LIST_DIRECTORIES false "include/*.hpp")
target_sources(${PROJECT_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS include FILES
  ${headers}
)

configure_file(cmake_src/build_version.hpp.in "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/build_version.hpp")
target_sources(${PROJECT_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS "${CMAKE_CURRENT_BINARY_DIR}/include" FILES
  "${CMAKE_CURRENT_BINARY_DIR}/include/${PROJECT_NAME}/build_version.hpp"
)

file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE
  ${sources}
)

target_precompile_headers(${PROJECT_NAME} PRIVATE
  <vulkan/vulkan.hpp>
)
