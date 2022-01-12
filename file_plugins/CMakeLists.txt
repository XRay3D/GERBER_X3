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

include (${PROJECT_SOURCE_DIR}/../SUFFIX.cmake)

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(SUFFIX "${SUFFIX}_x64")
    set(PLUGINS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bin_x64")
else()
    set(PLUGINS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../bin_x32")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SUFFIX "${SUFFIX}_d")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(SUFFIX "${SUFFIX}_r")
elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(SUFFIX "${SUFFIX}_rm")
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(SUFFIX "${SUFFIX}_rd")
endif()

set(PLUGINS_DIR "${PLUGINS_DIR}/plugins")


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
    ${CMAKE_CURRENT_SOURCE_DIR}/../libs${SUFFIX}
    )


link_directories(
    ../libs${SUFFIX}
    )

#add_subdirectory(dxf)
#add_subdirectory(excellon)
add_subdirectory(gerber)
#add_subdirectory(hpgl)



