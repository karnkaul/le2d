file(ARCHIVE_EXTRACT INPUT "${CMAKE_CURRENT_SOURCE_DIR}/src.zip" DESTINATION "${CMAKE_CURRENT_SOURCE_DIR}")

message(STATUS "[kvf]")
add_subdirectory(src/kvf)
message(STATUS "[djson]")
add_subdirectory(src/djson)
message(STATUS "[capo-lite]")
add_subdirectory(src/capo-lite)
