project(le2d-example)

function(add_exe name source)
  add_executable(${name})
  target_link_libraries(${name} le2d::le2d)
  target_sources(${name} PRIVATE ${source})
endfunction()

add_exe(example example.cpp)
