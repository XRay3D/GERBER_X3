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

project(GX3)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(SUFFIX "_clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(SUFFIX "_gnu")
    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}
        #        "-std=c++20"
        "-std=c++2a"
        #        "-std=gnu++20"
        )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(SUFFIX "_intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(SUFFIX "_msvc")
#    set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "/std:c++latest")
endif()


#if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
#    ...
#endif()
#if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
#    ...
#endif()

if(CMAKE_CL_64)
    set(SUFFIX "${SUFFIX}_x64")
else()
    set(SUFFIX "${SUFFIX}_x32")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SUFFIX "${SUFFIX}_d")
    message("${SUFFIX}")
endif()

set(CMAKE_GX3_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_PLUGINS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/bin/plugins)
set(CMAKE_STATIC_LIBS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/libs${SUFFIX})

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)

include_directories (GGEasy
    static_libs/clipper
    static_libs/common
    static_libs/filetree
    static_libs/gerber
    static_libs/gi
    static_libs/graphicsview
    static_libs/shape
    static_libs/thermal
    static_libs/tooldatabase
    )

# static lib
add_subdirectory(static_libs)

# main app
add_subdirectory(GGEasy)

# plugins file
add_subdirectory(file_plugins)

# plugins shape
add_subdirectory(shape_plugins)


