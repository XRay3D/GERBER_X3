#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  01 February 2020                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2021                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.14)

project(gerber LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PLUGINS_DIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_DIR})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Concurrent
    Core
    Gui
    Widgets
    REQUIRED)

include_directories(${CMAKE_STATIC_LIBS_DIR})

add_library(gerber SHARED
    gerber.json
    compdialog.ui
    compdialog.h
    compmodel.h
    compnode.h
    componentitem.h
    compview.h
    gbraperture.h
    gbrattraperfunction.h
    gbrattrfilefunction.h
    gbrattributes.h
    gbrcomponent.h
    gbrfile.h
    gbrnode.h
    gbrparser.h
    gbrplugin.h
    gbrtypes.h
    mathparser.h
    compdialog.cpp
    compmodel.cpp
    compnode.cpp
    componentitem.cpp
    compview.cpp
    gbraperture.cpp
    gbrattraperfunction.cpp
    gbrattrfilefunction.cpp
    gbrattributes.cpp
    gbrcomponent.cpp
    gbrfile.cpp
    gbrnode.cpp
    gbrparser.cpp
    gbrplugin.cpp
    mathparser.cpp
    )

target_link_libraries(gerber PRIVATE
    clipper
    common
    filetree
    gi
    graphicsview
    thermal
    tooldatabase
    )

target_link_libraries(gerber PRIVATE
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

target_compile_definitions(gerber PRIVATE GERBER_LIBRARY)
