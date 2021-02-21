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

project(dxf LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PLUGINS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_DIR})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

add_subdirectory(entities)
add_subdirectory(section)
add_subdirectory(tables)

add_library(dxf SHARED
    dxf_block.cpp
    dxf_block.h
    dxf_codedata.cpp
    dxf_codedata.h
    dxf_file.cpp
    dxf_file.h
    dxf_node.cpp
    dxf_node.h
    dxf_plugin.cpp
    dxf_plugin.h
    dxf_sourcedialog.cpp
    dxf_sourcedialog.h
    dxf_types.cpp
    dxf_types.h
    dxf.json
    ${HEADERS}
    ${SOURCE}
    )

link_directories(../../libs${SUFFIX})
target_link_libraries(dxf PRIVATE
    clipper
    common
    filetree
    gi
    graphicsview
    tooldatabase
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

target_compile_definitions(dxf PRIVATE DXF_LIBRARY)
