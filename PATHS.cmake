cmake_minimum_required(VERSION 3.20)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
set (CMAKE_PREFIX_PATH "C:/Qt/5.15.2/mingw81_64")# why???
endif()

find_package(QT NAMES Qt6 Qt5 Core)
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
else()
    set(DIR_NAME "${DIR_NAME}_x32")
endif()

set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${DIR_NAME}")

set(PLUGINS_DIR "${OUTPUT_DIRECTORY}/plugins")

set(STATIC_LIBS_DIR "${CMAKE_SOURCE_DIR}/bin/libs${DIR_NAME}")

message(${PROJECT_NAME})
message("1    ${CMAKE_SOURCE_DIR}")
message("2    ${STATIC_LIBS_DIR}")
message("3    ${PLUGINS_DIR}")
message("4    ${DIR_NAME}")

link_directories(STATIC_LIBS_DIR)
