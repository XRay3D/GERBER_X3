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
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PLUGINS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_DIR})
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

link_directories(../../libs${SUFFIX})
target_link_libraries(gerber PRIVATE
    clipper
    common
    filetree
    gi
    graphicsview
    thermal
    tooldatabase
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

#target_compile_definitions(gerber PRIVATE GERBER_LIBRARY)
