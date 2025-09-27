file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src.zip" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

if(NOT TARGET djson::djson)
  message(STATUS "[djson]")
  add_subdirectory(src/djson)
endif()

if(NOT TARGET capo::capo)
  message(STATUS "[capo-lite]")
  add_subdirectory(src/capo-lite)
endif()

message(STATUS "[kvf]")
add_subdirectory(src/kvf)
