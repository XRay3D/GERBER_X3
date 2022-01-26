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

project(File_Plugins)
set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include (${CMAKE_SOURCE_DIR}/SUFFIX.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${STATIC_LIBS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${STATIC_LIBS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_DIR})


find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)

include_directories (
    ../GGEasy
    ../compile-time-regular-expressions/single-header/
    ../magic_get/
    ../magic_get/include/
    ../static_libs/clipper
    ../static_libs/common
    ../static_libs/filetree
    ../static_libs/gerber
    ../static_libs/gi
    ../static_libs/graphicsview
    ../static_libs/shape
    ../static_libs/thermal
    ../static_libs/tooldatabase
    ../static_libs/tooldatabase
    ${CMAKE_SOURCE_DIR}/libs${SUFFIX}
    )


link_directories(
    ${CMAKE_SOURCE_DIR}/libs${SUFFIX}
    )

add_subdirectory(dxf)
add_subdirectory(excellon)
add_subdirectory(gerber)
#add_subdirectory(hpgl)



