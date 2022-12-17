#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  03 October 2022                                                 *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2022                                          *
#* License   :                                                                  *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#********************************************************************************/
cmake_minimum_required(VERSION 3.20)

set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

project(GERBER_X3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

add_definitions(
    -DQT_MESSAGELOGCONTEXT# qDebug()
    -DCTRE
    -DQT_DISABLE_DEPRECATED_BEFORE=0x060000
)
#add_definitions("-D__cpp_consteval")# source_location


if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        -Winvalid-pch
#        -fmodules-ts
    )
# set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FORMAT "gcc")
# set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG "${compiler_flags_for_module_map} -fmodule-mapper=<MODULE_MAP_FILE>")
# target_compile_options(example PRIVATE -fmodules-ts)
# add_compile_options(-H)
# add_compile_options(-ftime-report)
# -pthread
# -ltbbD
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

find_package(Boost)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    message(${Boost_INCLUDE_DIR})
    add_definitions( "-D_USE_BOOST_" )
else()
    message(FATAL_ERROR "Boost not found!!!")
endif()

include_directories(
    GGEasy
    external_dependencies/compile-time-regular-expressions/single-header
    external_dependencies/pfr_non_boost/include
    plugins/file
    plugins/gcode
    plugins/shape
    plugins/shape/shape
    static_libs/clipper
    static_libs/common
    static_libs/filetree
    static_libs/gi
    static_libs/graphicsview
    static_libs/tooldatabase
)

include(${CMAKE_SOURCE_DIR}/PATHS.cmake)

# plugins
add_subdirectory(static_libs)

# plugins
add_subdirectory(plugins)

# main app
add_subdirectory(GGEasy)

#target_precompile_headers(GGEasy
#    #    PUBLIC
#    #    qt.h
#    PRIVATE
#    #    [["other_header.h"]]
#    <QtWidgets>
#    <QtCore>
#    <forward_list>
#    <list>
#    <map>
#    <ranges>
#    <span>
#    <unordered_map>
#    <vector>
#)

