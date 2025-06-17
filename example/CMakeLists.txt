project(le2d-example)

add_executable(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
  le2d::le2d
)

target_sources(${PROJECT_NAME} PRIVATE
  example.cpp
)
