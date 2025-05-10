#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  03 October 2022
 Website   :  na
 Copyright :  Damir Bakiev 2016-2022
 License   :
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 3.20)

project(file LANGUAGES CXX)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE JSONS ${CMAKE_CURRENT_SOURCE_DIR}/*.json)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE UIS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

include_directories(${CMAKE_STATIC_LIBS_DIR})

add_library(${PROJECT_NAME} SHARED
    ${HEADERS}
    ${JSONS}
    ${SOURCES}
    ${UIS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    clipper
    common
    filetree
    gcode
    gi
    graphicsview
    tooldatabase
    file_plugins
    Qt${QT_VERSION_MAJOR}::Widgets
)



#target_compile_definitions(excellon PRIVATE EXCELLON_LIBRARY)
