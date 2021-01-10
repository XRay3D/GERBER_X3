cmake_minimum_required(VERSION 3.14)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_STATIC_LIBS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_STATIC_LIBS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_STATIC_LIBS_DIR})

# static lib
add_subdirectory(clipper)
add_subdirectory(filetree)
add_subdirectory(gi)
add_subdirectory(graphicsview)
add_subdirectory(common)
add_subdirectory(thermal)
add_subdirectory(tooldatabase)
add_subdirectory(shape)
