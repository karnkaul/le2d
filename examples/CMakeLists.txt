project(le2d-example)

function(add_exe name sources)
  add_executable(${name})
  target_link_libraries(${name} le2d::le2d)
  target_sources(${name} PRIVATE ${sources})
endfunction()

add_exe(quickstart quickstart.cpp)

add_subdirectory(demo)
