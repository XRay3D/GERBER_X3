#[[
 Author    :  Damir Bakiev                                                    
 Version   :  na                                                              
 Date      :  03 October 2024                                                 
 Website   :  na                                                              
 Copyright :  Damir Bakiev 2016-2024                                          
 License   :                                                                  
 Use, modification & distribution is subject to Boost Software License Ver 1. 
 http://www.boost.org/LICENSE_1_0.txt                                         
]]
cmake_minimum_required(VERSION 3.20)

project(gcpocketoffset LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(PLUG_NAME   "Pocket Offset")
set(PLUG_INFO   "Pocket Offset")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "0")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
    ${CMAKE_CURRENT_SOURCE_DIR}/description.json NEWLINE_STYLE UNIX
)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE JSONS ${CMAKE_CURRENT_SOURCE_DIR}/*.json)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE UIS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

include_directories(${CMAKE_STATIC_LIBS_DIR})

set(PROJECT_SOURCES
    ${HEADERS}
    ${JSONS}
    ${SOURCES}
    ${UIS}
)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    clipper
    common
    filetree
    gcode
    gi
    graphicsview
    tooldatabase
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

add_library(gc_pocketoffset STATIC
    pocketoffset.h
    pocketoffset.cpp
)

target_link_libraries(gc_pocketoffset PRIVATE
    clipper
    gcode
    common
    filetree
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

