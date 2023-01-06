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


project(clipper LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui REQUIRED)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/Clipper2Lib/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/Clipper2Lib/*.cpp)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Clipper2Lib/include)

add_library(clipper STATIC
    #    cpline.cpp
    #    cpline.h
    #    clipper.cpp
    #    clipper.hpp
    #    clipper_base.hpp
    #    clipper_clipper.hpp
    #    clipper_offset.hpp
    #    clipper_base.cpp
    #    clipper_clipper.cpp
    #    clipper_offset.cpp

    ${HEADERS}
    ${SOURCES}
    myclipper.cpp
    myclipper.h
)

target_link_libraries(clipper PRIVATE
    gcode
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
)

