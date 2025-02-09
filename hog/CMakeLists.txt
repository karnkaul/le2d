project(hog)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  le2d::le2d
  klib::klib-warnings
  klib::klib-ndwerror
  klib::klib-msbuild-mp
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src
)

target_precompile_headers(${PROJECT_NAME} REUSE_FROM le2d)

file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/**.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE
  ${sources}
)
