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

project(shpolyline LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(PLUG_NAME   "Poly Line")
set(PLUG_INFO   "Poly Line")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "1")
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
    ${CMAKE_CURRENT_SOURCE_DIR}/description.json NEWLINE_STYLE UNIX
)

find_package(QT NAMES Qt5 #[[Qt6]] COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets OpenGL REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS OpenGLWidgets REQUIRED)
endif()

file(GLOB PROJECT_SOURCES
    *.json
    *.cpp
    *.h
)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE
    clipper
    common
    filetree
    gi
    graphicsview
    shape
    tooldatabase
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::OpenGL
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

