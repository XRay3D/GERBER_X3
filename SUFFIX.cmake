cmake_minimum_required(VERSION 3.16)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(SUFFIX "_clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(SUFFIX "_gnu")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(SUFFIX "_intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(SUFFIX "_msvc")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SUFFIX "${SUFFIX}_d")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(SUFFIX "${SUFFIX}_r")
elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(SUFFIX "${SUFFIX}_rm")
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(SUFFIX "${SUFFIX}_rd")
endif()

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/x64")
else()
    set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/x32")
endif()

set(OUTPUT_DIRECTORY "${OUTPUT_DIRECTORY}${SUFFIX}")

set(PLUGINS_DIR "${OUTPUT_DIRECTORY}/plugins")

set(STATIC_LIBS_DIR "${CMAKE_SOURCE_DIR}/bin/libs${SUFFIX}")

message(${PROJECT_NAME})
message("    ${CMAKE_SOURCE_DIR}")
message("    ${STATIC_LIBS_DIR}")
message("    ${PLUGINS_DIR}")
message("    ${SUFFIX}")

link_directories(STATIC_LIBS_DIR)
