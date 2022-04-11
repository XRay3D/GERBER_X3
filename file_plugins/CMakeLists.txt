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
#set_property(GLOBAL PROPERTY CXX_STANDARD 20)
#set(CMAKE_CXX_STANDARD 20)


include (${CMAKE_SOURCE_DIR}/SUFFIX.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${STATIC_LIBS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${STATIC_LIBS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_DIR})

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
#find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
#    Core
#    Gui
#    Widgets
#    REQUIRED
#    )

include_directories (
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/libs${SUFFIX}
    ../GGEasy
    ../compile-time-regular-expressions/single-header
    ../magic_get
    ../magic_get/include
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
    )

link_directories(
    ${CMAKE_SOURCE_DIR}/libs${SUFFIX}
    )

#file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
#file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

#add_library(${PROJECT_NAME} STATIC
#    ${SOURCES}
#    ${HEADERS}
#    )

#target_link_libraries(${PROJECT_NAME} PRIVATE
##    clipper
##    common
##    filetree
##    gi
##    graphicsview
##    shape
##    thermal
##    tooldatabase
#    #    Qt${QT_VERSION_MAJOR}::Concurrent
#    Qt${QT_VERSION_MAJOR}::Core
#    Qt${QT_VERSION_MAJOR}::Gui
##    Qt${QT_VERSION_MAJOR}::OpenGL
##    Qt${QT_VERSION_MAJOR}::PrintSupport
##    Qt${QT_VERSION_MAJOR}::Widgets
#    )

add_subdirectory(dxf)
add_subdirectory(excellon)
add_subdirectory(gerber)
#add_subdirectory(hpgl)



