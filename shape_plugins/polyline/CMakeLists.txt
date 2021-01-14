#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  14 January 2021                                                 *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2021                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.14)

project(polyline LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)



set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_DIR})

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets OpenGL REQUIRED)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS OpenGLWidgets REQUIRED)
endif()

include_directories(
    ../../GGEasy
    ../../static_libs/clipper
    ../../static_libs/common
    ../../static_libs/filetree
    ../../static_libs/gerber
    ../../static_libs/gi
    ../../static_libs/graphicsview
    ../../static_libs/thermal
    ../../static_libs/tooldatabase
    ../shape
    )

link_directories(../../libs${SUFFIX})

add_library(polyline SHARED
    polyline.json
    shpolyline.cpp
    shpolyline.h
    )

target_link_libraries(polyline PRIVATE
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
    target_link_libraries(polyline PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

target_compile_definitions(polyline PRIVATE POLYLINE_LIBRARY)
