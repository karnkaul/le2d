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

if(LE2D_BUILD_SPIRV2CPP OR LE2D_BUILD_ASSED)
  message(STATUS "[clap]")
  add_subdirectory(src/clap)
endif()
