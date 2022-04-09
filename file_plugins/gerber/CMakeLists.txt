#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  01 February 2020                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2022                                          *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.16)

project(gerber LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

include_directories(${CMAKE_STATIC_LIBS_DIR})

add_library(${PROJECT_NAME} SHARED
    gerber.json
    compdialog.cpp
    compdialog.h
    compitem.cpp
    compitem.h
    compmodel.cpp
    compmodel.h
    compnode.cpp
    compnode.h
    compview.cpp
    compview.h
    gbraperture.cpp
    gbraperture.h
    gbrattraperfunction.cpp
    gbrattraperfunction.h
    gbrattrfilefunction.cpp
    gbrattrfilefunction.h
    gbrattributes.cpp
    gbrattributes.h
    gbrcomponent.cpp
    gbrcomponent.h
    gbrfile.cpp
    gbrfile.h
    gbrh.cpp
    gbrh.h
    gbrnode.cpp
    gbrnode.h
    gbrparser.cpp
    gbrparser.h
    gbrplugin.cpp
    gbrplugin.h
    gbrtypes.h
    mathparser.cpp
    mathparser.h
    )

target_link_libraries(${PROJECT_NAME} PRIVATE
    clipper
    common
    filetree
    gi
    graphicsview
#    thermal
    tooldatabase
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

#target_compile_definitions(gerber PRIVATE GERBER_LIBRARY)
