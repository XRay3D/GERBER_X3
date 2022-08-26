cmake_minimum_required(VERSION 3.16)

find_package(QT NAMES Qt6 Qt5)
set(DIR_NAME "Qt${QT_VERSION_MAJOR}.${QT_VERSION_MINOR}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(DIR_NAME "${DIR_NAME}_clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(DIR_NAME "${DIR_NAME}_gnu")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(DIR_NAME "${DIR_NAME}_intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(DIR_NAME "${DIR_NAME}_msvc")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(DIR_NAME "${DIR_NAME}_d")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(DIR_NAME "${DIR_NAME}_r")
elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(DIR_NAME "${DIR_NAME}_rm")
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(DIR_NAME "${DIR_NAME}_rd")
endif()

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(DIR_NAME "${DIR_NAME}_x64")
    #    set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/x64")
else()
    set(DIR_NAME "${DIR_NAME}_x32")
    #    set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/x32")
endif()

set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${DIR_NAME}")

set(PLUGINS_DIR "${OUTPUT_DIRECTORY}/plugins")

set(STATIC_LIBS_DIR "${CMAKE_SOURCE_DIR}/bin/libs${DIR_NAME}")

message(${PROJECT_NAME})
message("    ${CMAKE_SOURCE_DIR}")
message("    ${STATIC_LIBS_DIR}")
message("    ${PLUGINS_DIR}")
message("    ${DIR_NAME}")

link_directories(STATIC_LIBS_DIR)
