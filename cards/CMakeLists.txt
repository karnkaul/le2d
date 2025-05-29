cmake_minimum_required(VERSION 3.24)

project(cards)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  le2d::le2d
)

target_include_directories(${PROJECT_NAME} PRIVATE
  src
)

file(GLOB_RECURSE sources LIST_DIRECTORIES false "src/*.[hc]pp")
target_sources(${PROJECT_NAME} PRIVATE ${sources})

target_precompile_headers(${PROJECT_NAME} REUSE_FROM le2d)
