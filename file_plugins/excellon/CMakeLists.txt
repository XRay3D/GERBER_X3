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

project(excellon LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_PLUGINS_DIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_PLUGINS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_PLUGINS_DIR})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

include_directories(${CMAKE_STATIC_LIBS_DIR})

add_library(excellon SHARED
    excellondialog.ui
    drillitem.h
    excellon.h
    excellondialog.h
    exfile.h
    exnode.h
    exparser.h
    explugin.h
    extypes.h
    drillitem.cpp
    excellondialog.cpp
    exfile.cpp
    exformatstate.cpp
    exnode.cpp
    exparser.cpp
    explugin.cpp
    excellon.json
    ${SOURCE}
    ${HEADERS}
    )


target_link_libraries(excellon PRIVATE
    clipper
    common
    filetree
    gi
    graphicsview
    tooldatabase
    )

target_link_libraries(excellon PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

target_compile_definitions(excellon PRIVATE EXCELLON_LIBRARY)
