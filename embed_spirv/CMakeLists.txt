project(embed-spirv)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} PRIVATE
  klib::klib
)

target_sources(${PROJECT_NAME} PRIVATE
  main.cpp
)
